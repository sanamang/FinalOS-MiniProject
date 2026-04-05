#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

#define PIPESIZE 32

struct bad_pipe {
  char data[PIPESIZE];
  uint nread;   // number of bytes read (total)
  uint nwrite;  // number of bytes written (total)
};

void
pipe_write(struct bad_pipe *pi, char ch)
{
  pi->data[pi->nwrite % PIPESIZE] = ch;
  pi->nwrite++;
}

int
pipe_read(struct bad_pipe *pi)
{
  // Empty when nread == nwrite
  if (pi->nread == pi->nwrite)
    return -1;

  int ch = (unsigned char)pi->data[pi->nread % PIPESIZE];
  pi->nread++;
  return (unsigned char)ch;
}

int
main(void)
{
  struct bad_pipe pipe;
  pipe.nread = 0;
  pipe.nwrite = 0;

  char ch;
  char last3[3] = {0, 0, 0}; 

  printf("Type text. Enter 'ok?' to stop and display buffer contents.\n\n");

  // Read from stdin
  while (read(0, &ch, 1) == 1) {
    last3[0] = last3[1];
    last3[1] = last3[2];
    last3[2] = ch;

    if (last3[0] == 'o' && last3[1] == 'k' && last3[2] == '?') {
      if (pipe.nwrite >= 2)
        pipe.nwrite -= 2;
      break; 
    }

    //bad pipe buffer
    pipe_write(&pipe, ch);
  }

  printf("\n\n--- Buffer contents (bad pipe) ---\n");

  // Read back everything currently in the pipe and print
  int r;
  while ((r = pipe_read(&pipe)) != -1) {
    printf("%c", r);
  }

  printf("\n");
  exit(0);
}