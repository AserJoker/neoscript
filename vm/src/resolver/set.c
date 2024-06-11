#include "resolver/set.h"
#include "astdef.h"
#include "engine/include/type.h"
#include "engine/include/type/array.h"
#include "engine/include/type/exception.h"
#include "engine/include/type/object.h"
#include "engine/include/typedef.h"
#include "engine/include/value.h"
#include "vm.h"
#include <stdlib.h>
neo_value neo_resolver_set(neo_vm vm, neo_ast node) {

  neo_context ctx = neo_vm_get_context(vm);
  if (!node->right || !node->left) {
    neo_context_throw(ctx, create_neo_exception(ctx, "Invalid operator set",
                                                NULL, node->start.filename,
                                                node->start.line,
                                                node->start.column));
  }
  if (node->left->type == NEO_AST_TYPE_MEMBER) {
    if (!node->left->right || !node->left->left) {
      neo_context_throw(ctx,
                        create_neo_exception(ctx, "Invalid operator member",
                                             NULL, node->left->start.filename,
                                             node->left->start.line,
                                             node->left->start.column));
    }
    neo_value parent = neo_vm_eval(vm, node->left->left);
    neo_value key = neo_vm_eval(vm, node->left->right);
    neo_value value = neo_vm_eval(vm, node->right);
    if (neo_value_get_type_name(parent) == NEO_TYPE_OBJECT) {
      cstring key_s = NULL;
      if (neo_value_convert(key, NEO_TYPE_STRING, &key_s)) {
        neo_object_set_field(ctx, parent, key_s, value);
        free(key_s);
        return value;
      } else {
        char msg[1024];
        sprintf(msg, "cannot get field with key type:%d",
                neo_value_get_type_name(key));
        neo_context_throw(
            ctx, create_neo_exception(ctx, "Invalid operator member", NULL,
                                      node->left->right->start.filename,
                                      node->left->right->start.line,
                                      node->left->right->start.column));
      }
    } else if (neo_value_get_type_name(parent) == NEO_TYPE_ARRAY) {
      double index = 0;
      if (neo_value_convert(key, NEO_TYPE_NUMBER, &index)) {
        neo_array_set_index(ctx, parent, index, value);
        return value;
      } else {
        char msg[1024];
        sprintf(msg, "cannot get field with key type:%d",
                neo_value_get_type_name(key));
        neo_context_throw(
            ctx, create_neo_exception(ctx, "Invalid operator member", NULL,
                                      node->left->right->start.filename,
                                      node->left->right->start.line,
                                      node->left->right->start.column));
      }
    }
  }
  neo_value left = neo_vm_eval(vm, node->left);
  neo_value right = neo_vm_eval(vm, node->right);
  neo_atom right_atom = neo_value_get_atom(right);
  neo_type type = neo_value_get_type(right);
  neo_type_hook *hook = neo_type_get_hook(type);
  if (hook->copy) {
    neo_value_set_atom(left, hook->copy(right_atom, hook->copy_arg));
  } else {
    neo_value_set_atom(left, right_atom);
  }
  return left;
}