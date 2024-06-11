#include "resolver/add.h"
#include "engine/include/type.h"
#include "engine/include/type/exception.h"
#include "engine/include/type/number.h"
#include "engine/include/type/string.h"
#include "engine/include/typedef.h"
#include "engine/include/value.h"
#include "vm.h"
#include <stdlib.h>
#include <string.h>
neo_value neo_resolver_add(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  if (!node->right) {
    neo_context_throw(ctx, create_neo_exception(ctx, "Invalid operator add",
                                                NULL, node->start.filename,
                                                node->start.line,
                                                node->start.column));
  }
  neo_value left = NULL;
  if (node->left) {
    left = neo_vm_eval(vm, node->left);
  }
  neo_value right = neo_vm_eval(vm, node->right);
  uint32_t right_type = neo_value_get_type_name(right);
  if (!left) {
    double val = 0;
    if (neo_value_convert(right, NEO_TYPE_NUMBER, &val)) {
      return create_neo_number(ctx, val);
    }
    cstring msg = NULL;
    if (neo_value_convert(right, NEO_TYPE_STRING, &msg)) {
      cstring buf = malloc(strlen(msg) + 2);
      sprintf(buf, "+%s", msg);
      free(msg);
      neo_value result = create_neo_string(ctx, buf);
      free(buf);
      return result;
    }
    char message[1024];
    sprintf(message, "Invalid operator add with type (%d)", right_type);
    neo_context_throw(
        ctx, create_neo_exception(ctx, message, NULL, node->start.filename,
                                  node->start.line, node->start.column));
  }
  double left_val = 0;
  double right_val = 0;
  if (neo_value_convert(left, NEO_TYPE_NUMBER, &left_val) &&
      neo_value_convert(right, NEO_TYPE_NUMBER, &right_val)) {
    return create_neo_number(ctx, left_val + right_val);
  }
  cstring left_str = NULL;
  cstring right_str = NULL;
  if (neo_value_convert(left, NEO_TYPE_STRING, &left_str) &&
      neo_value_convert(right, NEO_TYPE_STRING, &right_str)) {
    cstring buf = malloc(strlen(left_str) + strlen(right_str) + 1);
    sprintf(buf, "%s%s", left_str, right_str);
    free(left_str);
    free(right_str);
    neo_value result = create_neo_string(ctx, buf);
    free(buf);
    return result;
  }
  if (left_str) {
    free(left_str);
  }
  if (right_str) {
    free(right_str);
  }
  char message[1024];
  sprintf(message, "Invalid operator add with type (%d and %d)",
          neo_value_get_type_name(left), right_type);
  neo_context_throw(
      ctx, create_neo_exception(ctx, message, NULL, node->start.filename,
                                node->start.line, node->start.column));
  return NULL;
}