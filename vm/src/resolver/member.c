#include "resolver/member.h"
#include "engine/include/type.h"
#include "engine/include/type/array.h"
#include "engine/include/type/exception.h"
#include "engine/include/type/object.h"
#include "engine/include/typedef.h"
#include "engine/include/value.h"
#include "vm.h"
#include <stdlib.h>

neo_value neo_resolver_member(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  if (!node->right || !node->left) {
    neo_context_throw(ctx, create_neo_exception(ctx, "Invalid operator member",
                                                NULL, node->start.filename,
                                                node->start.line,
                                                node->start.column));
  }
  neo_value left = neo_vm_eval(vm, node->left);
  neo_value right = neo_vm_eval(vm, node->right);
  if (neo_value_get_type_name(left) == NEO_TYPE_OBJECT) {
    char *name = NULL;
    if (neo_value_convert(right, NEO_TYPE_STRING, &name)) {
      neo_value field = neo_object_get_field(ctx, left, name);
      free(name);
      return field;
    } else {
      char msg[1024];
      sprintf(msg, "cannot get field with key type:%d",
              neo_value_get_type_name(right));
      neo_context_throw(ctx,
                        create_neo_exception(ctx, "Invalid operator member",
                                             NULL, node->right->start.filename,
                                             node->right->start.line,
                                             node->right->start.column));
    }
  }
  if (neo_value_get_type_name(left) == NEO_TYPE_ARRAY) {
    double index = 0;
    if (neo_value_convert(right, NEO_TYPE_NUMBER, &index)) {
      return neo_array_get_index(ctx, left, index);
    }
  } else {
    char msg[1024];
    sprintf(msg, "cannot get field with key type:%d",
            neo_value_get_type_name(right));
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "Invalid operator member", NULL,
                                           node->right->start.filename,
                                           node->right->start.line,
                                           node->right->start.column));
  }
  return NULL;
}