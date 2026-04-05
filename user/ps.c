#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(2, "Usage: ps [-o | -l]\n");
    exit(1);
  }
  kps(argv[1]);
  exit(0);
}