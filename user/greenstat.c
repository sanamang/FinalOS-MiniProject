#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void) {
  printf("GreenX Energy Dashboard\n");
  printf("%-6s %-10s\n", "PID", "Ticks Used");
  printf("------  ----------\n");
  int total = 0;
  for(int pid = 1; pid <= 64; pid++) {
    int ticks = getpenergy(pid);
    if(ticks >= 0) {
      printf("%-6d %-10d\n", pid, ticks);
      total += ticks;
    }
  }
  printf("------  ----------\n");
  printf("%-6s %-10d\n", "TOTAL", total);
  exit(0);
}
