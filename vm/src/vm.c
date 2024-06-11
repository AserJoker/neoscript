#include "vm.h"
#include "compiler/include/astdef.h"
#include "common/include/imap.h"
#include "common/include/list.h"
#include "engine/include/type/exception.h"
#include "resolver/add.h"
#include "resolver/add_and_set.h"
#include "resolver/and.h"
#include "resolver/and_and_set.h"
#include "resolver/boolean.h"
#include "resolver/call.h"
#include "resolver/comma.h"
#include "resolver/dec.h"
#include "resolver/div.h"
#include "resolver/div_and_set.h"
#include "resolver/eq.h"
#include "resolver/gae.h"
#include "resolver/gt.h"
#include "resolver/inc.h"
#include "resolver/lae.h"
#include "resolver/le.h"
#include "resolver/left_shift.h"
#include "resolver/left_shift_and_set.h"
#include "resolver/logic_and.h"
#include "resolver/logic_not.h"
#include "resolver/logic_or.h"
#include "resolver/member.h"
#include "resolver/mod.h"
#include "resolver/mod_and_set.h"
#include "resolver/mul_and_set.h"
#include "resolver/ne.h"
#include "resolver/not.h"
#include "resolver/null.h"
#include "resolver/number.h"
#include "resolver/or.h"
#include "resolver/or_and_set.h"
#include "resolver/right_shift.h"
#include "resolver/right_shift_and_set.h"
#include "resolver/set.h"
#include "resolver/string.h"
#include "resolver/sub.h"
#include "resolver/sub_and_set.h"
#include "resolver/symbol.h"
#include "resolver/xor.h"
#include "resolver/xor_and_set.h"
#include <stdlib.h>
struct _neo_vm {
  neo_context ctx;
  neo_imap resolvers;
};
neo_vm create_neo_vm(neo_context ctx) {
  neo_vm vm = malloc(sizeof(struct _neo_vm));
  vm->resolvers = create_neo_imap((neo_free_fn)free_neo_list);
  vm->ctx = ctx;
  neo_vm_set_resolver(vm, NEO_AST_TYPE_NULL, neo_resolver_null);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_BOOLEAN, neo_resolver_boolean);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_NUMBER, neo_resolver_number);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_STRING, neo_resolver_string);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_SYMBOL, neo_resolver_symbol);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_ADD, neo_resolver_add);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_SUB, neo_resolver_sub);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_DIV, neo_resolver_div);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_MOD, neo_resolver_mod);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_INC, neo_resolver_inc);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_DEC, neo_resolver_dec);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_EQ, neo_resolver_eq);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_NE, neo_resolver_ne);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_GT, neo_resolver_gt);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_GAE, neo_resolver_gae);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_LE, neo_resolver_le);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_LAE, neo_resolver_lae);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_LOGIC_AND, neo_resolver_logic_and);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_LOGIC_OR, neo_resolver_logic_or);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_LOGIC_NOT, neo_resolver_logic_not);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_AND, neo_resolver_and);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_OR, neo_resolver_or);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_XOR, neo_resolver_xor);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_NOT, neo_resolver_not);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_LEFT_SHIFT, neo_resolver_left_shift);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_RIGHT_SHIFT, neo_resolver_right_shift);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_ADD_AND_SET, neo_resolver_add_and_set);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_SUB_AND_SET, neo_resolver_sub_and_set);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_DIV_AND_SET, neo_resolver_div_and_set);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_MUL_AND_SET, neo_resolver_mul_and_set);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_MOD_AND_SET, neo_resolver_mod_and_set);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_AND_AND_SET, neo_resolver_and_and_set);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_OR_AND_SET, neo_resolver_or_and_set);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_XOR_AND_SET, neo_resolver_xor_and_set);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_LEFT_SHIFT_AND_SET,
                      neo_resolver_left_shift_and_set);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_RIGHT_SHIFT_AND_SET,
                      neo_resolver_right_shift_and_set);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_COMMA, neo_resolver_comma);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_SET, neo_resolver_set);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_MEMBER, neo_resolver_member);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_CALL, neo_resolver_call);
  return vm;
}
neo_context neo_vm_get_context(neo_vm self) { return self->ctx; }
void neo_vm_set_context(neo_vm self, neo_context ctx) { self->ctx = ctx; }
void free_neo_vm(neo_vm vm) {
  free_neo_imap(vm->resolvers);
  free(vm);
}
neo_value neo_vm_eval(neo_vm self, neo_ast ast) {
  neo_list resolvers = neo_imap_get(self->resolvers, ast->type);
  if (resolvers) {
    neo_list_node node = neo_list_tail(resolvers);
    while (node != neo_list_head(resolvers)) {
      neo_vm_resolver resolver = neo_list_node_get(node);
      if (resolver) {
        neo_value result = resolver(self, ast);
        if (result) {
          return result;
        }
      }
      node = neo_list_node_last(node);
    }
  }
  neo_context_throw_exception(self->ctx, "cannot resolve node");
  return NULL;
}
void neo_vm_set_resolver(neo_vm self, int32_t type, neo_vm_resolver resolver) {
  if (!neo_imap_get(self->resolvers, type)) {
    neo_imap_set(self->resolvers, type, create_neo_list(NULL));
  }
  neo_list resolvers = neo_imap_get(self->resolvers, type);
  neo_list_push(resolvers, resolver);
}