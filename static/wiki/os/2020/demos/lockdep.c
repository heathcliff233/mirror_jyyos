#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdlib.h>

#define MAX_DEPTH   32
#define MAX_LOGS    1024
#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

typedef int (*func_t)(pthread_mutex_t *);

static func_t __pthread_mutex_lock   = NULL;
static func_t __pthread_mutex_unlock = NULL;
static pthread_mutex_t glk = PTHREAD_MUTEX_INITIALIZER;

// global lock ordering log
static struct lockdep {
  void *lk1, *lk2;
} lockdeps[MAX_LOGS];

// per-thread lock stack
static __thread void *lock_stack[MAX_DEPTH];
static __thread int lock_top = 0;

__attribute__((constructor)) static void init() {
  __pthread_mutex_lock   = (func_t)dlsym(RTLD_NEXT, "pthread_mutex_lock");
  __pthread_mutex_unlock = (func_t)dlsym(RTLD_NEXT, "pthread_mutex_unlock");
}

static void panic_on(int cond, const char *msg) {
  if (cond) {
    fprintf(stderr, "[lockdep] !!!panic!!! %s\n", msg);
    exit(1);
  }
}

static void lockdep_check(void *lk1, void *lk2) {
  int deadlock = 0;

  __pthread_mutex_lock(&glk); // BE CAREFUL!

  for (int i = 0; i < LENGTH(lockdeps); i++) {
    struct lockdep *d = &lockdeps[i];

    // known lockdep; okay
    if (d->lk1 == lk1 && d->lk2 == lk2) goto release;

    // AB BA; deadlock
    if (d->lk2 == lk1 && d->lk1 == lk2) deadlock = 1;

    // new lockdep; log it
    if (!d->lk1 && !d->lk2) {
      d->lk1 = lk1; d->lk2 = lk2;
      printf("[lockdep] new lock ordering: %p -> %p\n", lk1, lk2);
      goto release;
    }
  }

release:
  __pthread_mutex_unlock(&glk);
  panic_on(deadlock, "deadlock");
}

int pthread_mutex_lock(pthread_mutex_t *lk) {
  for (int i = 0; i < lock_top; i++)
    lockdep_check(lock_stack[i], lk);
  panic_on(lock_top >= MAX_DEPTH, "too deep held locks");
  lock_stack[lock_top++] = lk;

  return __pthread_mutex_lock(lk);
}

int pthread_mutex_unlock(pthread_mutex_t *lk) {
  lock_top--;
  panic_on(lock_top < 0, "dangling unlock");
  panic_on(lock_stack[lock_top] != lk, "unpaired unlock");

  return __pthread_mutex_unlock(lk);
}
