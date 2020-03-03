#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

void func() {
  printf("At exit\n");
}

int main() {
  atexit(func);

  int roll = (time(0) & 1) == 0;
  if (roll) {
    printf("Call _exit()\n");
    _exit(0);
  } else {
    printf("Call exit()\n");
    exit(0);
  }
}
