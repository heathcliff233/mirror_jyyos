#include <am.h>
#include <klib.h>

#define MAX_CPU 8
#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

typedef union task {
  struct {
    const char *name;
    union task *next;
    _Context   *context;
  };
  uint8_t stack[4096];
} Task;

struct cpu_local {
  Task *current;
} cpu_local[MAX_CPU];
#define current cpu_local[_cpu()].current

typedef struct {
  intptr_t locked;
} lock_t;

void lock(lock_t *lock) {
  while (_atomic_xchg(&lock->locked, 1));
}

void unlock(lock_t *lock) {
  _atomic_xchg(&lock->locked, 0);
}

Task tasks[] = {
  { .name = "A" },
  { .name = "B" },
  { .name = "C" },
  { .name = "D" },
  { .name = "E" },
};

_Context *on_interrupt(_Event ev, _Context *ctx) {
  if (!current) {
    current = &tasks[0];
  } else {
    current->context = ctx;
  }
  do {
    current = current->next;
  } while ((current - tasks) % _ncpu() != _cpu());
  return current->context;
}

lock_t biglock;

void func(void *arg) {
  while (1) {
    lock(&biglock);
    printf("Thread-%s on CPU #%d acquired the lock\n", arg, _cpu());
    unlock(&biglock);
    for (int volatile i = 0; i < 100000; i++) ;
  }
}

void mp_entry() {
  _intr_write(1);
  _yield();
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
  _mpe_init(mp_entry);
}
