#ifndef __NEO_ENGINE_SCOPE__
#define __NEO_ENGINE_SCOPE__

#include "util/list.h"
typedef struct _neo_scope *neo_scope;
typedef struct _neo_value *neo_value;
typedef struct _neo_atom *neo_atom;

neo_scope create_neo_scope(neo_scope parent);
void free_neo_scope(neo_scope scope);
neo_scope neo_scope_get_parent(neo_scope self);

void neo_scope_add_value(neo_scope self, neo_value value);
void neo_scope_remove_value(neo_scope self, neo_value value);
neo_atom neo_scope_get_root(neo_scope self);
#endif