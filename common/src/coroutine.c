#ifdef __unix__
#include "coroutine.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>
#ifndef NEO_COROUTINE_STACK_SIZE
#define NEO_COROUTINE_STACK_SIZE 4096
#endif

struct _neo_co_context {
  ucontext_t cpuinfo;
  char stack[NEO_COROUTINE_STACK_SIZE];
};

neo_coroutine neo_co_start(void (*func)(void *), void *arg) {
  neo_coroutine coroutine = malloc(sizeof(struct _neo_co_context));
  memset(coroutine, 0, sizeof(struct _neo_co_context));
  getcontext(&coroutine->cpuinfo);
  if (func) {
    coroutine->cpuinfo.uc_stack.ss_sp = coroutine->stack;
    coroutine->cpuinfo.uc_stack.ss_size = NEO_COROUTINE_STACK_SIZE;
    coroutine->cpuinfo.uc_stack.ss_flags = 0;
    makecontext(&coroutine->cpuinfo, (void (*)())func, 1, arg);
  }
  return coroutine;
}
void neo_co_yield(neo_coroutine current, neo_coroutine coroutine) {
  //   neo_co_switch(&current->cpuinfo, &coroutine->cpuinfo);
  swapcontext(&current->cpuinfo, &coroutine->cpuinfo);
}
void neo_co_destroy(neo_coroutine coroutine) {
  free(coroutine);
}
#endif