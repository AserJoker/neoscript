#ifndef __NEO_COMMON_COROUTINE__
#define __NEO_COMMON_COROUTINE__
#include <stdint.h>
typedef struct _neo_co_context *neo_coroutine;
neo_coroutine neo_co_start(void (*func)(void *), void *arg);
void neo_co_yield(neo_coroutine current, neo_coroutine coroutine);
void neo_co_destroy(neo_coroutine coroutine);
#endif