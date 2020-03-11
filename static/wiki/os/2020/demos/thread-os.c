#include <am.h>
#include <klib.h>

#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

typedef union task {
  struct {
    const char *name;
    union task *next;
    _Context   *context;
  };
  uint8_t stack[4096];
} Task;

Task *current = NULL, tasks[] = {
  { .name = "a" },
  { .name = "b" },
  { .name = "c" },
};

_Context *on_interrupt(_Event ev, _Context *ctx) {
  if (!current) {
    current = &tasks[0];     // first interrupt
  } else {
    current->context = ctx;  // interrupt context saved on stack
    current = current->next; // schedule the next context
  }
  return current->context;
}

void func(void *arg) {
  while (1) {
    printf("%s", arg);
    for (int volatile i = 0; i < 100000; i++);
  }
}

int main() {
  _ioe_init();
  _cte_init(on_interrupt);

  for (int i = 0; i < LENGTH(tasks); i++) {
    Task *task    = &tasks[i];
    _Area stack   = (_Area) { &task->context + 1, task + 1 };
    task->context = _kcontext(stack, func, (void *)task->name);
    task->next    = &tasks[(i + 1) % LENGTH(tasks)];
  }
  _intr_write(1);
  _yield();
}
