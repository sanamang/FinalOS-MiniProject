#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"

#define PIPESIZE 512

struct pipe {
  struct spinlock lock;
  char data[PIPESIZE];
  uint nread;     // number of bytes read
  uint nwrite;    // number of bytes written
  int readopen;   // read fd is still open
  int writeopen;  // write fd is still open
  volatile int flag[2];
  volatile int turn;
};

int
pipealloc(struct file **f0, struct file **f1)
{
  struct pipe *pi;

  pi = 0;
  *f0 = *f1 = 0;

  if((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0)
    goto bad;

  if((pi = (struct pipe*)kalloc()) == 0)
    goto bad;

  // init pipe state
  pi->readopen = 1;
  pi->writeopen = 1;
  pi->nwrite = 0;
  pi->nread = 0;

  pi->flag[0] = 0;
  pi->flag[1] = 0;
  pi->turn = 0;

  initlock(&pi->lock, "pipe");

  (*f0)->type = FD_PIPE;
  (*f0)->readable = 1;
  (*f0)->writable = 0;
  (*f0)->pipe = pi;

  (*f1)->type = FD_PIPE;
  (*f1)->readable = 0;
  (*f1)->writable = 1;
  (*f1)->pipe = pi;

  return 0;

bad:
  if(pi)
    kfree((char*)pi);
  if(*f0)
    fileclose(*f0);
  if(*f1)
    fileclose(*f1);
  return -1;
}


void
pipeclose(struct pipe *pi, int writable)
{
  acquire(&pi->lock);
  if(writable){
    pi->writeopen = 0;
    wakeup(&pi->nread);
  } else {
    pi->readopen = 0;
    wakeup(&pi->nwrite);
  }
  if(pi->readopen == 0 && pi->writeopen == 0){
    release(&pi->lock);
    kfree((char*)pi);
  } else
    release(&pi->lock);
}

static inline void
peterson_enter(struct pipe *pi, int me)
{
  int other = 1 - me;
  pi->flag[me] = 1;
  pi->turn = other;
  while(pi->flag[other] && pi->turn == other)
    ;
}

static inline void
peterson_exit(struct pipe *pi, int me)
{
  pi->flag[me] = 0;
}

int
pipewrite(struct pipe *pi, uint64 addr, int n)
{
  int i = 0;
  struct proc *pr = myproc();

  // REPLACE: acquire(&pi->lock);
  peterson_enter(pi, 0); // Writer is process 0 

  while(i < n){
    if(pi->readopen == 0 || killed(pr)){
      peterson_exit(pi, 0);
      return -1;
    }
    
    // Check if buffer is full
    if(pi->nwrite == pi->nread + PIPESIZE){
       // Note: Peterson's is a busy-wait. 
       // In this specific lab, we usually busy-wait or 
       // briefly yield instead of using the original sleep().
       continue; 
    } else {
      char ch;
      if(copyin(pr->pagetable, &ch, addr + i, 1) == -1)
        break;
      pi->data[pi->nwrite % PIPESIZE] = ch;
      pi->nwrite++;
      i++;
    }
  }

  // REPLACE: release(&pi->lock);
  peterson_exit(pi, 0);
  return i;
}

int
piperead(struct pipe *pi, uint64 addr, int n)
{
  int i;
  struct proc *pr = myproc();
  char ch;

  // REPLACE: acquire(&pi->lock);
  peterson_enter(pi, 1); // Reader is process 1 

  // If empty and write end is still open, wait
  while(pi->nread == pi->nwrite && pi->writeopen){
    if(killed(pr)){
      peterson_exit(pi, 1);
      return -1;
    }
    // Busy wait for data
  }

  for(i = 0; i < n; i++){
    if(pi->nread == pi->nwrite)
      break;
    ch = pi->data[pi->nread % PIPESIZE];
    if(copyout(pr->pagetable, addr + i, &ch, 1) == -1) {
      if(i == 0) i = -1;
      break;
    }
    pi->nread++;
  }

  // REPLACE: release(&pi->lock);
  peterson_exit(pi, 1);
  return i;
}