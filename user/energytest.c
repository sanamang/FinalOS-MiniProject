#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void fail(const char *msg) {
  printf("FAIL: %s\n", msg);
  exit(1);
}

void pass(const char *msg) {
  printf("PASS: %s\n", msg);
}

int main(int argc, char *argv[])
{
  int pid, energy1, energy2, st;

  printf("Starting Energy Test Suite...\n");

  // Test 1: seturgency bounds
  if(seturgency(0) != 0) fail("seturgency(0) failed");
  if(seturgency(1) != 0) fail("seturgency(1) failed");
  if(seturgency(2) != 0) fail("seturgency(2) failed");
  if(seturgency(99) != -1) fail("seturgency(99) should fail");
  pass("Test 1: seturgency bounds");

  // Test 2: getpenergy tracks busy loop and handles invalid PID
  energy1 = getpenergy(getpid());
  int i, j = 0;
  for(i=0; i<10000000; i++) { j += i; } // Burn some ticks
  energy2 = getpenergy(getpid());
  if(energy2 < energy1) fail("getpenergy returned lower value after busy loop");
  if(getpenergy(9999) != -1) fail("getpenergy(9999) should return -1");
  pass("Test 2: getpenergy basic functionality");

  // Test 3: sleep and exit cleanly
  pid = fork();
  if(pid < 0) fail("Test 3 fork failed");
  if(pid == 0) {
    pause(20);
    exit(0);
  }
  wait(&st);
  if(st != 0) fail("Test 3 child exit not 0");
  pass("Test 3: slept/exit cleanly");

  // Test 4: setbudget kills process
  pid = fork();
  if(pid < 0) fail("Test 4 fork failed");
  if(pid == 0) {
    if(setbudget(3) != 0) fail("Test 4 setbudget failed");
    while(1) {} // Spin forever, should be killed
  }
  wait(&st);
  if(st != -1) fail("Test 4 child exit status should be -1 for killed by budget");
  pass("Test 4: budget killed spinning process");

  // Test 5: low urgency can still getpenergy
  pid = fork();
  if(pid < 0) fail("Test 5 fork failed");
  if(pid == 0) {
    if(seturgency(0) != 0) fail("Test 5 seturgency failed");
    int e = getpenergy(getpid());
    if(e < 0) fail("Test 5 low urgency getpenergy failed");
    exit(0);
  }
  wait(&st);
  if(st != 0) fail("Test 5 child exit not 0");
  pass("Test 5: low urgency processes work properly");

  // Test 6: exec greenstat
  pid = fork();
  if(pid < 0) fail("Test 6 fork failed");
  if(pid == 0) {
    char *args[] = {"greenstat", 0};
    exec("greenstat", args);
    fail("exec greenstat failed");
  }
  wait(&st);
  if(st != 0) fail("Test 6 exec greenstat exit status not 0");
  pass("Test 6: exec greenstat succeeded");

  printf("All tests completed.\n");
  exit(0);
}
