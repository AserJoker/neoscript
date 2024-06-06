#include "strings.h"
#include "list.h"
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
int8_t strings_compare(const char *s1, const char *s2) {
  return strcmp(s1, s2) == 0;
}
char *strings_join(neo_list list, const char *spliter) {
  size_t current = 128;
  char *buf = malloc(current);
  size_t len = 0;
  neo_list_node node = neo_list_head(list);
  while (node != neo_list_tail(list)) {
    char *line = neo_list_node_get(node);
    if (line) {
      len += strlen(line);
      if (len >= current) {
        current += 128;
        char *newbuf = malloc(current);
        strcpy(newbuf, buf);
        free(buf);
        buf = newbuf;
      }
      strcat(buf,line);
    }
    node = neo_list_node_next(node);
    if (node != neo_list_tail(list)) {
      len += strlen(spliter);
      if (len >= current) {
        current += 128;
        char *newbuf = malloc(current);
        strcpy(newbuf, buf);
        free(buf);
        buf = newbuf;
      }
      strcat(buf,spliter);
    }
  }
  return buf;
}