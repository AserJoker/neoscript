#include "common/include/coroutine.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
neo_coroutine co_task = NULL;
neo_coroutine co_main = NULL;
int flag = 0;
void task(void *arg) {
  int data = *(int *)arg;
  char s[1024];
  flag += data;
  neo_co_yield(co_task, co_main);
}
int main() {
  int val = 123;
  co_main = neo_co_start(NULL, NULL);
  co_task = neo_co_start(&task, &val);
  assert(flag == 0);
  neo_co_yield(co_main, co_task);
  assert(flag == val);
  return 0;
}