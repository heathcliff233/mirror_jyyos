#include <am.h>

void say(const char *s) {
  for (; *s; s++) _putc(*s);
}

int main() {
  say("hello\n");
  return 0;
}
