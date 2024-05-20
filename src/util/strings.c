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