#include <am.h>
#include <klib.h>

#define LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

struct task {
  const char *name;
  _Context context;
  char stack[4096];
} tasks[] = {
  { "task-1" },
  { "task-2" },
  { "task-3" },
};

struct task *current = NULL;

_Context *interrupt(_Event ev, _Context *ctx) {
  if (current) current->context = *ctx; // save context

  if (!current || current + 1 == &tasks[LENGTH(tasks)]) {
    current = &tasks[0]; // back to the first task
  } else {
    current++;
  }

  printf("\nSchedule: %s\n", current->name);

  return &current->context;
}

void func(void *arg) {
  int cur = (intptr_t)arg;
  while (1) {
    printf("%c", "abcdefghijk"[cur]);
  }
}

int main() {
  _ioe_init();
  _cte_init(interrupt);

  for (int i = 0; i < LENGTH(tasks); i++) {
    struct task *task = &tasks[i];
    _Area stack = (_Area) { task->stack, task + 1 };
    task->context = *_kcontext(stack, func, (void *)i);
  }
  _intr_write(1);
  while (1);
}
