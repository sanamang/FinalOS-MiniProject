#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "stat.h"
#include "proc.h"

// Use a sleeplock so begin_op() can sleep without panic.
// Must only be called from process context (not interrupt handlers).
static struct sleeplock greenlog_lock;
static int greenlog_ready = 0;

void
greenlog_init(void)
{
  initsleeplock(&greenlog_lock, "greenlog");
  greenlog_ready = 1;
}

static void
itoa_k(int n, char *buf, int *pos)
{
  char tmp[12];
  int i = 0;
  if(n < 0){ buf[(*pos)++] = '-'; n = -n; }
  if(n == 0){ buf[(*pos)++] = '0'; return; }
  while(n > 0){ tmp[i++] = '0' + (n % 10); n /= 10; }
  while(i > 0){ buf[(*pos)++] = tmp[--i]; }
}

static void
appends(char *buf, int *pos, char *s)
{
  while(*s) buf[(*pos)++] = *s++;
}

// Append msg to /greenlog — must be called from process context only.
void
greenlog_write(char *msg)
{
  struct inode *ip, *dp;
  uint off;
  int len;

  if(!greenlog_ready)
    return;

  len = strlen(msg);
  if(len == 0)
    return;

  acquiresleep(&greenlog_lock);
  begin_op();

  ip = namei("/greenlog");
  if(ip == 0){
    dp = namei("/");
    if(dp == 0){ end_op(); releasesleep(&greenlog_lock); return; }
    ilock(dp);

    ip = dirlookup(dp, "greenlog", 0);
    if(ip == 0){
      ip = ialloc(dp->dev, T_FILE);
      if(ip == 0){ iunlock(dp); iput(dp); end_op(); releasesleep(&greenlog_lock); return; }
      ilock(ip);
      ip->nlink = 1;
      iupdate(ip);
      if(dirlink(dp, "greenlog", ip->inum) < 0){
        ip->nlink = 0;
        iupdate(ip);
        iunlockput(ip);
        iunlock(dp);
        iput(dp);
        end_op();
        releasesleep(&greenlog_lock);
        return;
      }
    } else {
      ilock(ip);
    }
    iunlock(dp);
    iput(dp);
  } else {
    ilock(ip);
  }

  off = ip->size;
  writei(ip, 0, (uint64)msg, off, len);
  iunlockput(ip);
  end_op();

  releasesleep(&greenlog_lock);
}

void
greenlog_budget_exceeded(int pid, char *name, uint64 ticks_used, uint64 budget)
{
  char buf[128];
  int pos = 0;
  appends(buf, &pos, "[BUDGET_EXCEEDED] pid=");
  itoa_k(pid, buf, &pos);
  appends(buf, &pos, " name=");
  appends(buf, &pos, name);
  appends(buf, &pos, " ticks=");
  itoa_k((int)ticks_used, buf, &pos);
  appends(buf, &pos, " budget=");
  itoa_k((int)budget, buf, &pos);
  buf[pos++] = '\n';
  buf[pos]   = 0;
  greenlog_write(buf);
}

void
greenlog_urgency_change(int pid, char *name, int new_level)
{
  char buf[128];
  int pos = 0;
  char *level_str = (new_level == 0) ? "LOW" :
                    (new_level == 1) ? "NORMAL" : "HIGH";
  appends(buf, &pos, "[URGENCY_CHANGE] pid=");
  itoa_k(pid, buf, &pos);
  appends(buf, &pos, " name=");
  appends(buf, &pos, name);
  appends(buf, &pos, " level=");
  appends(buf, &pos, level_str);
  buf[pos++] = '\n';
  buf[pos]   = 0;
  greenlog_write(buf);
}
