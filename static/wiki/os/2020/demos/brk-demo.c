#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

void *get_brk() {
  return sbrk(0);
}

void _system(const char *fmt, ...) {
  char *cmd;
  va_list args;
  va_start(args, fmt);
  vasprintf(&cmd, fmt, args);
  if (cmd) {
    system(cmd);
    free(cmd);
  }
}

int main() {
  extern char end;

  _system("cat /proc/%d/maps", getpid());

  printf("End   at %p\n", &end);
  printf("Break at %p\n", get_brk());
  sbrk(0x4000000); // 64 MiB
  printf("Break at %p (after malloc)\n", get_brk());
}
