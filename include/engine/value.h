#ifndef __NEO_ENGINE_VALUE_
#define __NEO_ENGINE_VALUE_
#include "engine/atom.h"
#include "engine/context.h"
typedef struct _neo_value *neo_value;

neo_value create_neo_value(neo_context ctx, neo_atom atom);
void free_neo_value(neo_value value);

neo_atom neo_value_get_atom(neo_value self);

neo_value neo_value_assign(neo_value self, neo_value another);
#endif