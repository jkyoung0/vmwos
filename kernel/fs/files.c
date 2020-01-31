#include <stdint.h>
#include <stddef.h>

#include "lib/errors.h"
#include "lib/printk.h"
#include "lib/string.h"
#include "lib/smp.h"

#include "drivers/console/console_io.h"

#include "fs/files.h"
#include "fs/inodes.h"
#include "fs/superblock.h"

#include "processes/process.h"

static int debug=0;

static struct file_object file_objects[MAX_OPEN_FILES];

int32_t file_object_free(struct file_object *file) {

	/* FIXME: locking */

	/* Free the corresponding inode */
	if (file->inode) inode_free(file->inode);

	/* Reduce the count */
	if (file->count) file->count--;

	return 0;
}

int32_t file_object_allocate(struct inode_type *inode) {

	int32_t index;

	if (debug) printk("Attempting to allocate fd for inode %x\n",
		inode->number);

	index=0;
	while(1) {
		if (file_objects[index].count==0) {
			file_objects[index].count=1;
			file_objects[index].inode=inode;
			file_objects[index].file_offset=0;
			break;
		}
		index++;
		if (index>=MAX_OPEN_FILES) {
			index=-ENFILE;
			break;
		}
	}

	if (debug) printk("### Allocated file %d\n",index);

	return index;
}

int32_t close_syscall(uint32_t fd) {

	int32_t result;

	result=file_object_free(&file_objects[fd]);

	return result;

}

struct inode_type *file_get_inode(int32_t which) {

	return file_objects[which].inode;
}

/****************************************************/
/* open                                             */
/****************************************************/

int32_t open_file(const char *pathname, uint32_t flags, uint32_t mode) {

	int32_t result;
	struct inode_type *inode;

	if (debug) {
		printk("### Trying to open %s\n",pathname);
	}

	result=inode_lookup_and_alloc(pathname,&inode);
	if (result<0) {
		return -ENOENT;
	}

	if (debug) {
		printk("\tFound inode %x\n",inode->number);
	}

	result=file_object_allocate(inode);
	if (result<0) {
		return result;
	}

	/* Set up the file_ops */
	inode->sb->sb_ops.setup_fileops(&file_objects[result]);

	if (debug) printk("### opened fd %d\n",result);

	return result;
}

/****************************************************/
/* open file object                                 */
/****************************************************/

int32_t open_file_object(
	struct file_object **file,
	const char *pathname, uint32_t flags, uint32_t mode) {

	int32_t result;

	result=open_file(pathname,flags,mode);
	if (result<0) {
		return result;
	}

	*file=&file_objects[result];

	return 0;
}

/****************************************************/
/* open syscall                                     */
/****************************************************/

int32_t open_syscall(const char *pathname, uint32_t flags, uint32_t mode) {

	int32_t result;

	result=open_file(pathname,flags,mode);

	return result;

}

int32_t read_syscall(uint32_t fd, void *buf, uint32_t count) {

	int32_t result;
	struct file_object *file;

	/* Hack for now */
	if (fd==0) {
		result=console_read(buf,count);
		return result;
	}

	if (fd>=MAX_OPEN_FILES) {
		return -ENFILE;
	}

	file=&file_objects[fd];

	if (file->count==0) {
		printk("Attempting to read from unopened fd %d\n",fd);
		return -EBADF;
	}

	if (debug) printk("Attempting to read %d bytes from fd %d into %x\n",count,fd,buf);

	result=file->file_ops->read(
				file_objects[fd].inode->sb,
				file_objects[fd].inode->number,
				buf,count,
				&file_objects[fd].file_offset);

	return result;
}

int32_t write_syscall(uint32_t fd, void *buf, uint32_t count) {

	int32_t result;
	struct file_object *file;

	/* Hack */
	if ((fd==1) || (fd==2)) {
		result = console_write(buf, count);
		return result;
	}

	if (fd>=MAX_OPEN_FILES) {
		return -ENFILE;
	}

	file=&file_objects[fd];

	if (file->count==0) {
		printk("Attempting to write to unopened fd %d\n",fd);
		return -EBADF;
	}

	result=file->file_ops->write(
				file_objects[fd].inode->sb,
				file_objects[fd].inode->number,
					buf,count,
					&file_objects[fd].file_offset);

	return result;
}

void file_objects_init(void) {
	int i;

	for(i=0;i<MAX_OPEN_FILES;i++) {
		file_objects[i].count=0;
	}

	/* Special case 0/1/2 (stdin/stdout/stderr) */
	/* FIXME: actually hook them up to be proper file objects */
	file_objects[0].count=1;
	file_objects[1].count=1;
	file_objects[2].count=1;

	return;
}

int32_t getdents_syscall(uint32_t fd,
			struct vmwos_dirent *dirp, uint32_t count) {

	int result;
	struct file_object *file;

	if (fd>=MAX_OPEN_FILES) {
		return -ENFILE;
	}

	file=&file_objects[fd];

	if (file->count==0) {
		printk("Attempting to getdents from unsupported fd %d\n",fd);
		return -EBADF;
	}

	/* FIXME: check if it's a directory fd */
	if (debug) {
	}

	result=file->file_ops->getdents(
					file_objects[fd].inode->sb,
					file_objects[fd].inode->number,
					&(file_objects[fd].file_offset),
					dirp,count);
	return result;

}

/* Change current working directory */
/* FIXME: should make path canonical somehow */
int32_t chdir_syscall(const char *path) {

	int32_t result,len;
	struct inode_type *inode;
	char new_path[MAX_PATH_LEN];

	/* See if too big */
	len=strlen(path);
	if (len>MAX_PATH_LEN) {
		return -E2BIG;
	}

	/* expand path */

	/* Absolute */
        if (path[0]=='/') {
                strncpy(new_path,path,MAX_PATH_LEN);
        }
	/* Relative */
        else {
                /* prepend old path */
                snprintf(new_path,MAX_PATH_LEN,"%s/%s",
                        current_proc[get_cpu()]->current_dir,
                        path);
        }

	/* Strip off trailing slashes if not in root */
	if ((len>1) && (new_path[len-1]=='/')) {
		len--;
		new_path[len]='\0';
	}

	/* Check to see if destination exists */
	result=inode_lookup_and_alloc(new_path,&inode);
	if (result<0) {
		return -ENOENT;
	}

	if ((inode->mode&S_IFMT)!=S_IFDIR) {
		inode_free(inode);
		return -ENOTDIR;
	}

	/* Things check out, copy over old path */
	strncpy(current_proc[get_cpu()]->current_dir,new_path,MAX_PATH_LEN);

	/* TODO: keep a pointer to the inode in the current dir? */
	inode_free(inode);
	return 0;
}


/* Get name of current working directory */
char *getcwd_syscall(char *buf, size_t size) {


	strncpy(buf,current_proc[get_cpu()]->current_dir,size);

	return buf;

}

int64_t llseek_generic(struct file_object *file,
		int64_t offset, int32_t whence) {

	if (file==NULL) return -EBADF;

	switch(whence) {
		case SEEK_SET:
			file->file_offset=offset;
			break;
		case SEEK_CUR:
			file->file_offset+=offset;
			break;
		case SEEK_END:
			/* FIXME: need to get size from inode */
			return -ENOSYS;
			break;
		default:
			return -EINVAL;
	}

	return file->file_offset;
}


int64_t llseek_syscall(uint32_t fd, int64_t offset, int32_t whence) {

	int64_t result;

	result=file_objects[fd].file_ops->llseek(&file_objects[fd],
							offset,whence);

	return result;
}
