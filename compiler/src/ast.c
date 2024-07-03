#include "ast.h"
#include "common/include/allocator.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

neo_ast_meta make_neo_ast(enum NEO_AST_TYPE type, neo_token start,
                          neo_token end) {
  neo_ast_meta meta = {type, start, end};
  return meta;
}

neo_ast *make_neo_ast_list(neo_allocator allocator, neo_ast node, ...) {
  size_t len = 1;
  neo_ast *arr = NULL;
  if (!node) {
    arr = malloc(sizeof(neo_ast));
    arr[0] = 0;
    return arr;
  }
  va_list list;
  va_start(list, node);
  neo_ast item = va_arg(list, neo_ast);
  len++;
  while (item) {
    item = va_arg(list, neo_ast);
    len++;
  }
  va_end(list);
  arr = neo_malloc(allocator, sizeof(neo_ast) * (len + 1));
  arr[0] = node;
  size_t index = 1;

  va_start(list, node);
  item = va_arg(list, neo_ast);
  while (item) {
    arr[index++] = item;
    item = va_arg(list, neo_ast);
  }
  va_end(list);
  arr[len] = 0;
  return arr;
}