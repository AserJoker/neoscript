#ifndef __NEO_ENGINE_ATOM__
#define __NEO_ENGINE_ATOM__
#include "type.h"
typedef struct _neo_atom *neo_atom;
neo_atom create_neo_atom(neo_type type, void *value);
void free_neo_atom(neo_atom atom);

void neo_atom_set(neo_atom self, void *data);
void *neo_atom_get(neo_atom self);

void neo_atom_add_ref(neo_atom self, neo_atom parent);
void neo_atom_remove_ref(neo_atom self, neo_atom parent);

neo_type neo_atom_get_type(neo_atom self);
neo_atom neo_atom_copy_val(neo_atom src, void *_);
neo_atom neo_atom_copy_ref(neo_atom src, void *_);
#endif