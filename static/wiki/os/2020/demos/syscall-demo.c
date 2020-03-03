#include <unistd.h>
#include <sys/syscall.h>

#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

const char hello[] = "\033[01;31mHello, OS World\033[0m\n";
// try: const char *hello = ...;

int main() {
  syscall(SYS_write, 1, hello, LENGTH(hello));
  syscall(SYS_exit, 1);
}
