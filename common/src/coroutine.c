#include "coroutine.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#ifndef NEO_COROUTINE_STACK_SIZE
#define NEO_COROUTINE_STACK_SIZE 4096
#endif
typedef struct _neo_cpuinfo *neo_cpuinfo;
struct _neo_cpuinfo {
#ifdef __x86_64__
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rdi;
  uint64_t rsi;
  uint64_t rsp;
  uint64_t rbp;
  uint64_t rip;
#endif
};

struct _neo_co_context {
  struct _neo_cpuinfo cpuinfo;
  char stack[NEO_COROUTINE_STACK_SIZE];
};
static void neo_co_switch(neo_cpuinfo src, neo_cpuinfo dst) {
#ifdef __x86_64__
  asm volatile("movq %%rax,0(%%rdi)\n\t"
               "movq %%rbx,8(%%rdi)\n\t"
               "movq %%rcx,16(%%rdi)\n\t"
               "movq %%rdx,24(%%rdi)\n\t"
               "movq %%rdi,32(%%rdi)\n\t"
               "movq %%rsi,40(%%rdi)\n\t"
               "movq %%rbp,%%rbx\n\t"
               "add $16,%%rbx\n\t"
               "movq %%rbx,48(%%rdi)\n\t"
               "movq 0(%%rbp),%%rbx\n\t"
               "movq %%rbx,56(%%rdi)\n\t"
               "movq 8(%%rbp),%%rbx\n\t"
               "movq %%rbx,64(%%rdi)\n\t"
               "movq 0(%%rsi),%%rax\n\t"
               "movq 16(%%rsi),%%rcx\n\t"
               "movq 24(%%rsi),%%rdx\n\t"
               "movq 48(%%rsi),%%rsp\n\t"
               "movq 56(%%rsi),%%rbp\n\t"
               "movq 64(%%rsi),%%rbx\n\t"
               "pushq %%rbx\n\t"
               "movq 8(%%rsi),%%rbx\n\t"
               "movq 32(%%rsi),%%rdi\n\t"
               "movq 40(%%rsi),%%rsi\n\t"
               "ret\n\t" ::
                   :);
#endif
}

neo_coroutine neo_co_start(void (*func)(void *), void *arg) {
  neo_coroutine coroutine = malloc(sizeof(struct _neo_co_context));
  memset(coroutine, 0, sizeof(struct _neo_co_context));
  coroutine->cpuinfo.rsp = (ptrdiff_t)coroutine->stack + 4096;
  coroutine->cpuinfo.rbp = (ptrdiff_t)coroutine->stack + 4096;
  coroutine->cpuinfo.rip = (ptrdiff_t)func;
  coroutine->cpuinfo.rdi = (ptrdiff_t)arg;
  return coroutine;
}
void neo_co_yield(neo_coroutine current, neo_coroutine coroutine) {
  neo_co_switch(&current->cpuinfo, &coroutine->cpuinfo);
}