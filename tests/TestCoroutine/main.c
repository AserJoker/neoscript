#include "coroutine.h"
#include <stddef.h>
#include <stdio.h>
neo_coroutine co_task = NULL;
neo_coroutine co_main = NULL;
void task(void *arg) {
  int data = *(int *)arg;
  while (1) {
    char s[1024];
    sprintf(s, "hello world,%d", data);
    printf("%s\n", s);
    neo_co_yield(co_task, co_main);
  }
}
int main() {
  int val = 123;
  co_main = neo_co_start(NULL, NULL);
  co_task = neo_co_start(&task, &val);
  while (1) {
    printf("co_main\n");
    neo_co_yield(co_main, co_task);
  }
  return 0;
}