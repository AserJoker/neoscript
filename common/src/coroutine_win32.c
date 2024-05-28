#ifdef _WIN32
#include "coroutine.h"
#include <windows.h>

#ifndef NEO_COROUTINE_STACK_SIZE
#define NEO_COROUTINE_STACK_SIZE 4096
#endif
struct _neo_co_context {
  LPVOID fiber;
};
neo_coroutine neo_co_start(void (*func)(void *), void *arg) {
  neo_coroutine coroutine = malloc(sizeof(struct _neo_co_context));
  memset(coroutine, 0, sizeof(struct _neo_co_context));
  if (!func) {
    coroutine->fiber = ConvertThreadToFiber(NULL);
  } else {
    coroutine->fiber = CreateFiber(NEO_COROUTINE_STACK_SIZE, func, arg);
  }
  return coroutine;
}
void neo_co_yield(neo_coroutine current, neo_coroutine coroutine) {
  SwitchToFiber(coroutine->fiber);
}
void neo_co_destroy(neo_coroutine coroutine) {
  if (coroutine->fiber!=GetCurrentFiber()) {
    DeleteFiber(coroutine->fiber);
  }
  free(coroutine);
}
#endif