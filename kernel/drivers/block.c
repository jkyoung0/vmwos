#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/string.h"

#include "drivers/drivers.h"
#include "drivers/block.h"



static struct block_dev_type block_devs[BLOCK_DEV_MAX];

static int32_t num_block_devs=0;

struct block_dev_type *allocate_block_dev(void) {

	if (num_block_devs==BLOCK_DEV_MAX-1) {
		return NULL;
	}

	num_block_devs++;

	return &block_devs[num_block_devs];
}

struct block_dev_type *block_dev_find(const char *name) {

	int i;

	for(i=0;i<BLOCK_DEV_MAX;i++) {
		if (!strncmp(name,block_devs[i].name,strlen(name))) {
			return &block_devs[i];
		}
	}

	return NULL;
}