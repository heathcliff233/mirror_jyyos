#include <am.h>
#include <klib.h>
#include <klib-macros.h>

// Trivial Lab 1: Physical Memory Management
//   "malloc" without free
void *kalloc(size_t size) {
  static uintptr_t brk = 0;
  brk = brk ? ROUNDUP(brk, size) + size : (uintptr_t)_heap.start + size;
  return (void *)(brk - size);
}
void kfree(void *ptr) { }

// Trivial Lab 2: Kernel Multithreading
//   a trivial round-robin thread (context) scheduler
typedef union task {
  struct {
    _Context *context;
    _AddressSpace vm;
    char *page, *cmd;
  };
  uint8_t stack[8192];
} Task;
Task tasks[] = {
  { .cmd = "/bin/counter" },
  { .cmd = "/bin/counter" },
  { .cmd = "/bin/counter" },
  { .cmd = "/bin/hello" },
  { .cmd = "/bin/hello" },
}, **currents;
#define current currents[_cpu()]

typedef intptr_t lock_t;
void spin_lock(lock_t *lock)   { _intr_write(0); while (_atomic_xchg(lock, 1)); }
void spin_unlock(lock_t *lock) { _atomic_xchg(lock, 0); _intr_write(1); }

_Context *schedule(_Context *ctx) {
  if (!current) current = &tasks[0];
  else current->context = ctx;
  do {
    if (++current == tasks + LENGTH(tasks)) {
      current = &tasks[0];
    }
  } while ((current - tasks) % _ncpu() != _cpu());
  return current->context;
}

// Trivial Lab 3: Virtual File System
//   a filesystem consisting of only preloaded files
void vfs_read(const char *fname, char *buf) {
  static char bin_hello[] = {
#include "apps/hello.inc"
  };
  static char bin_counter[] = {
#include "apps/counter.inc"
  };
  if (strcmp(fname, "/bin/hello") == 0)   memcpy(buf, bin_hello, sizeof(bin_hello));
  if (strcmp(fname, "/bin/counter") == 0) memcpy(buf, bin_counter, sizeof(bin_counter));
}

// Trivial Lab 4: Processes and Systemcalls
//   processes of only one file-mapped page; one syscall: puts
void proc_create(Task *task) {
  _Area stack = (_Area) { &task->context + 1, task + 1 };
  _AddressSpace *as = &task->vm;
  _protect(as);
  task->context = _ucontext(as, stack, as->area.start);
}

void proc_pagefault(void *ptr) {
  _AddressSpace *as = &current->vm;
  void *va = (void *)ROUNDDOWN(ptr, as->pgsize);
  void *pa = kalloc(as->pgsize);
  current->page = pa;
  vfs_read(current->cmd, pa);
  _map(as, va, pa, _PROT_READ | _PROT_WRITE | _PROT_EXEC);
}

void proc_syscall(_Context *ctx) {
  static lock_t lock = 0;
  void *pa = current->page + (ctx->GPR1 & (current->vm.pgsize - 1));
  spin_lock(&lock);
  printf("[syscall@P%d on cpu-%d] puts '%s'\n", current - tasks + 1, _cpu(), pa);
  spin_unlock(&lock);
}

// Entries
void os_init() {
  currents = kalloc(_ncpu() * sizeof(Task *));
  for (int i = 0; i < _ncpu(); i++)
    currents[i] = NULL;
  for (int i = 0; i < LENGTH(tasks); i++)
    proc_create(&tasks[i]);
}

_Context *os_interrupt(_Event ev, _Context *ctx) {
  switch (ev.event) {
    case _EVENT_SYSCALL:
      _intr_write(1);
      proc_syscall(ctx);
      return ctx;
    case _EVENT_PAGEFAULT:
      proc_pagefault((void *)ev.ref);
      return ctx;
    default:
      return schedule(ctx);
  }
}

int main() {
  // TODO: compile multilpe ABIs
#ifndef __ARCH_X86_64_QEMU
#error "Only supports x86_64-qemu (programs will load but with wrong ABI)"
#endif
  _ioe_init();
  _cte_init(os_interrupt);
  _vme_init(kalloc, kfree);
  os_init();
  _mpe_init(_yield);
}
