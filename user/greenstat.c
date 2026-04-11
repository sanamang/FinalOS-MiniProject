#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int pid;
  int energy;
  int count = 0;
  int total = 0;

  printf("GreenX Energy Report\n");
  printf("--------------------\n");
  printf("%-6s %-10s\n", "PID", "TICKS");

  for(pid = 1; pid <= NPROC; pid++) {
    energy = getpenergy(pid);
    if(energy != -1) {
      printf("%-6d %-10d\n", pid, energy);
      count++;
      total += energy;
    }
  }

  printf("--------------------\n");
  printf("Processes: %d  |  Total ticks: %d\n", count, total);

  exit(0);
}
