/* ctype.c */
int32_t iscntrl(int32_t c);
int32_t isdigit(int32_t c);
int32_t isspace(int32_t c);

/* div.c */
uint32_t __aeabi_uidiv(uint32_t dividend, uint32_t divisor);
int32_t __aeabi_idiv(int32_t dividend, int32_t divisor);

int32_t __aeabi_idivmod(int32_t dividend, int32_t divisor);

/* error.c */
extern int errno;
char *strerror(int errnum);

/* stdio.c */
#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

struct file_struct {
	int fd;
};
typedef struct file_struct FILE;
FILE stdin,stdout,stderr;

int putchar(int c);
int puts(char *s);
int getchar(void);
int sscanf(const char *str, const char *format, ...);

FILE *fopen(const char *pathname, const char *mode);
char *fgets(char *s, int size, FILE *stream);
int fclose(FILE *stream);

int32_t getline(char **lineptr, size_t *n, FILE *stream);

/* malloc.c */
void *malloc(uint32_t size);
void free(void *ptr);
void *calloc(uint32_t nmemb, uint32_t size);
void *realloc(void *ptr, uint32_t size);

/* printf.c */
int printf(const char *string,...);
int sprintf(char *string, char *fmt, ...);
int snprintf(char *str, uint32_t size, const char *format, ...);
int fprintf(FILE *stream, const char *string, ...);
int vsnprintf(char *str, uint32_t size, const char *format, ...);

/* random.c */
int32_t rand(void);

/* string.c */
int32_t atoi(char *string);
int strlen(const char *s);
int strncmp(const char *s1, const char *s2, uint32_t n);
char *strncpy(char *dest, const char *src, uint32_t n);
void *memset(void *s, int c, uint32_t n);
void *memcpy(void *dest, const void *src, uint32_t n);
int32_t memcmp(const void *s1, const void *s2, uint32_t n);
void *memmove(void *dest, const void *src, size_t n);
char *strchr(const char *s, int32_t c);
char *strrchr(const char *s, int32_t c);
char *strstr(const char *s1, const char *s2);
char *strdup(const char *s);

/* system.c */
int reboot(void);

/* time.c */
int32_t sleep(uint32_t seconds);
int32_t usleep(uint32_t usecs);
int time(int *time);
char *time_pretty(int32_t time, char *buffer, int32_t size);

/* tty.c */
void cfmakeraw(struct termios *termios_p);
