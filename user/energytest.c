#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void test_urgency(void) {
  printf("TEST: seturgency\n");
  if(seturgency(0) == 0) printf("  PASS: set LOW urgency\n");
  else printf("  FAIL: set LOW urgency\n");
  if(seturgency(1) == 0) printf("  PASS: set NORMAL urgency\n");
  else printf("  FAIL: set NORMAL urgency\n");
  if(seturgency(3) == -1) printf("  PASS: invalid urgency rejected\n");
  else printf("  FAIL: invalid urgency should return -1\n");
}

void test_energy_metering(void) {
  printf("TEST: getpenergy\n");
  int before = getpenergy(getpid());
  for(volatile int i = 0; i < 1000000; i++);
  int after = getpenergy(getpid());
  if(after >= before) printf("  PASS: ticks increased (%d -> %d)\n", before, after);
  else printf("  FAIL: ticks did not increase\n");
  if(getpenergy(9999) == -1) printf("  PASS: unknown PID returns -1\n");
  else printf("  FAIL: unknown PID should return -1\n");
}

void test_budget(void) {
  printf("TEST: setbudget\n");
  if(setbudget(0) == 0) printf("  PASS: unlimited budget accepted\n");
  else printf("  FAIL: unlimited budget\n");
  if(setbudget(-1) == -1) printf("  PASS: negative budget rejected\n");
  else printf("  FAIL: negative budget should return -1\n");
  int pid = fork();
  if(pid == 0) {
    setbudget(10);
    for(volatile int i = 0;;i++);
    exit(0);
  } else {
    int status;
    wait(&status);
    printf("  PASS: budget-limited child terminated by kernel\n");
  }
}

int main(void) {
  printf("=== GreenX Energy Test Suite ===\n");
  test_urgency();
  test_energy_metering();
  test_budget();
  printf("=== Tests Complete ===\n");
  exit(0);
}
