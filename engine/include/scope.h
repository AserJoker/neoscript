#ifndef __NEO_ENGINE_SCOPE__
#define __NEO_ENGINE_SCOPE__

typedef struct _neo_scope *neo_scope;
typedef struct _neo_value *neo_value;
typedef struct _neo_atom *neo_atom;

neo_scope create_neo_scope(neo_scope parent);
void free_neo_scope(neo_scope scope);
neo_scope neo_scope_get_parent(neo_scope self);

void neo_scope_add_value(neo_scope self, neo_value value);
void neo_scope_remove_value(neo_scope self, neo_value value);
neo_atom neo_scope_get_root(neo_scope self);

neo_value neo_scope_clone_value(neo_scope self, neo_value value);

void neo_scope_store_value(neo_scope self, const char *name, neo_value value);
neo_value neo_scope_load_value(neo_scope self, const char *name);
neo_value neo_scope_query_value(neo_scope self, const char *name);
#endif