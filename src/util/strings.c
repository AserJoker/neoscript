#include "util/strings.h"
#include <stdlib.h>
#include <string.h>

char *strings_clone(const char *str) {
  if (!str) {
    return NULL;
  }
  size_t len = strlen(str);
  char *buf = malloc(len + 1);
  strcpy(buf, str);
  buf[len] = 0;
  return buf;
}
char *strings_from_int(int val) {
  char *buf = malloc(32);
  buf[0] = 0;
  char *ptr = buf;
  while (val > 0) {
    *ptr = val % 10 + '0';
    val /= 10;
    ptr++;
  }
  *ptr = 0;
  size_t len = strlen(buf);
  for (int i = 0; i < len / 2; i++) {
    char swap = buf[i];
    buf[i] = buf[len - i - 1];
    buf[len - i - 1] = swap;
  }
  return buf;
}