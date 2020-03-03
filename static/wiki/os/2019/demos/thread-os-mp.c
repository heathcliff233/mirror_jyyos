#include <am.h>
#include <klib.h>

#define LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

struct task {
  const char *name;
  _Context context;
  char stack[4096];
} tasks[] = {
  { "task-1 @ cpu0" },
  { "task-2 @ cpu1" },
  { "task-3 @ cpu0" },
  { "task-4 @ cpu1" },
};

struct task *current_task[8];

#define current (current_task[_cpu()])

_Context *interrupt(_Event ev, _Context *ctx) {
  if (current) current->context = *ctx;

  do {
    if (!current || current + 1 == &tasks[LENGTH(tasks)]) {
      current = &tasks[0];
    } else {
      current++;
    }
  } while ((current - tasks) % _ncpu() != _cpu());

  printf("\n[cpu-%d] Schedule: %s\n", _cpu(), current->name);

  return &current->context;
}

void func(void *arg) {
  int cur = (intptr_t)arg;
  while (1) {
    printf("%c ", "123456789a"[cur]);
    for (int volatile i = 0; i < 10000; i++);
  }
}

void mp_entry() {
  _intr_write(1);
  while (1);
}

int main() {
  _ioe_init();
  _cte_init(interrupt);

  for (int i = 0; i < LENGTH(tasks); i++) {
    struct task *task = &tasks[i];
    _Area stack = (_Area) { task->stack, task + 1 };
    task->context = *_kcontext(stack, func, (void *)i);
  }

  _mpe_init(mp_entry);
}

