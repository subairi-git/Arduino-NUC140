#include <sys/types.h>
#include <errno.h>
#include <stddef.h>
extern char _end;
extern unsigned int _estack;
static char *heap_end;
void *_sbrk(ptrdiff_t incr) {
  char *prev=heap_end ? heap_end : &_end;
  // guard 1 KB of stack
  if(prev+incr >= (char *)&_estack - 1024) { errno=ENOMEM; return (void*)-1; }
  heap_end=prev+incr;
  return prev;
}
int _write(int fd,const void *buf,size_t len){ (void)fd;(void)buf;(void)len; errno=ENOSYS;return -1; }
int _close(int fd){(void)fd;return -1;}
int _fstat(int fd,void *st){(void)fd;(void)st;return -1;}
int _isatty(int fd){(void)fd;return 0;}
int _lseek(int fd,int offset,int whence){(void)fd;(void)offset;(void)whence;return -1;}
int _read(int fd,void *buf,size_t len){(void)fd;(void)buf;(void)len;return -1;}
int _kill(int pid,int sig){(void)pid;(void)sig;return -1;}
int _getpid(void){return 1;}
