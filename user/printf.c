#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#include <stdarg.h>

static char digits[] = "0123456789ABCDEF";

static void
putc(int fd, char c)
{
  write(fd, &c, 1);
}

static void
printint(int fd, long long xx, int base, int sgn, int width, int left_align)
{
  char buf[20];
  int i, neg;
  unsigned long long x;

  neg = 0;
  if(sgn && xx < 0){
    neg = 1;
    x = -xx;
  } else {
    x = xx;
  }

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);
  if(neg)
    buf[i++] = '-';

  int len = i;
  if(!left_align){
    while(width > len){
      putc(fd, ' ');
      width--;
    }
  }
  while(--i >= 0)
    putc(fd, buf[i]);
  if(left_align){
    while(width > len){
      putc(fd, ' ');
      width--;
    }
  }
}

static void
printstr(int fd, char *s, int width, int left_align)
{
  int len = 0;
  char *p = s;
  while(*p++) len++;

  if(!left_align){
    while(width > len){
      putc(fd, ' ');
      width--;
    }
  }
  for(p=s; *p; p++)
    putc(fd, *p);
  if(left_align){
    while(width > len){
      putc(fd, ' ');
      width--;
    }
  }
}

static void
printptr(int fd, uint64 x) {
  int i;
  putc(fd, '0');
  putc(fd, 'x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    putc(fd, digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the given fd. Only understands %d, %x, %p, %c, %s.
void
vprintf(int fd, const char *fmt, va_list ap)
{
  char *s;
  int c0, c1, c2, i, state;
  int width, left_align;

  state = 0;
  for(i = 0; fmt[i]; i++){
    c0 = fmt[i] & 0xff;
    if(state == 0){
      if(c0 == '%'){
        state = '%';
        width = 0;
        left_align = 0;
      } else {
        putc(fd, c0);
      }
    } else if(state == '%'){
      if(c0 == '-'){
        left_align = 1;
        continue;
      }
      if(c0 >= '0' && c0 <= '9'){
        width = width * 10 + (c0 - '0');
        continue;
      }

      c1 = c2 = 0;
      if(c0) c1 = fmt[i+1] & 0xff;
      if(c1) c2 = fmt[i+2] & 0xff;

      if(c0 == 'd'){
        printint(fd, va_arg(ap, int), 10, 1, width, left_align);
      } else if(c0 == 'l' && c1 == 'd'){
        printint(fd, va_arg(ap, uint64), 10, 1, width, left_align);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
        printint(fd, va_arg(ap, uint64), 10, 1, width, left_align);
        i += 2;
      } else if(c0 == 'u'){
        printint(fd, va_arg(ap, uint32), 10, 0, width, left_align);
      } else if(c0 == 'l' && c1 == 'u'){
        printint(fd, va_arg(ap, uint64), 10, 0, width, left_align);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'u'){
        printint(fd, va_arg(ap, uint64), 10, 0, width, left_align);
        i += 2;
      } else if(c0 == 'x'){
        printint(fd, va_arg(ap, uint32), 16, 0, width, left_align);
      } else if(c0 == 'l' && c1 == 'x'){
        printint(fd, va_arg(ap, uint64), 16, 0, width, left_align);
        i += 1;
      } else if(c0 == 'l' && c1 == 'l' && c2 == 'x'){
        printint(fd, va_arg(ap, uint64), 16, 0, width, left_align);
        i += 2;
      } else if(c0 == 'p'){
        printptr(fd, va_arg(ap, uint64));
      } else if(c0 == 'c'){
        putc(fd, va_arg(ap, uint32));
      } else if(c0 == 's'){
        if((s = va_arg(ap, char*)) == 0)
          s = "(null)";
        printstr(fd, s, width, left_align);
      } else if(c0 == '%'){
        putc(fd, '%');
      } else {
        putc(fd, '%');
        putc(fd, c0);
      }
      state = 0;
    }
  }
}

void
fprintf(int fd, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vprintf(fd, fmt, ap);
}

void
printf(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vprintf(1, fmt, ap);
}
