#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int p[2];
  if(pipe(p) < 0){
    fprintf(2, "pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if(pid < 0){
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if(pid == 0){
    // child: reader
    close(p[1]); // close write end
    char buf[64];
    int n;

    while((n = read(p[0], buf, sizeof(buf))) > 0){
      write(1, buf, n); // print to stdout
    }

    close(p[0]);
    exit(0);
  } else {
    // parent: writer
    close(p[0]); // close read end

    char *haiku =
      "On the phone, my grandfathers voice\n"
      "so frail\n"
      "No, my father's\n";

    write(p[1], haiku, strlen(haiku));
    close(p[1]);   // important: makes child read() return 0 at end
    wait(0);
    exit(0);
  }
}
