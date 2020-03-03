#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))
pthread_t *threads[128];
void (*cleanup_func)();

static void join_all() {
  for (int i = 0; i < LENGTH(threads); i++) {
    pthread_t *t = threads[i];
    if (t) {
      pthread_join(*t, NULL);
      free(t);
    }
  }
  if (cleanup_func) {
    cleanup_func();
  }
}

static void *entry_all(void *f) {
  ((void (*)())f)();
  return NULL;
}

static void create(void (*func)()) {
  for (int i = 0; i < LENGTH(threads); i++) {
    if (threads[i] == NULL) {
      if (i == 0) {
        atexit(join_all);
      }
      pthread_t *t = malloc(sizeof(pthread_t));
      threads[i] = t;
      pthread_create(t, NULL, entry_all, func);
      break;
    }
  }
}

static void join(void (*func)()) {
  cleanup_func = func;
}
