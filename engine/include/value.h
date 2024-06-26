#ifndef __NEO_ENGINE_VALUE_
#define __NEO_ENGINE_VALUE_
#include "atom.h"
#include "type.h"
typedef struct _neo_value *neo_value;
typedef struct _neo_scope *neo_scope;

neo_value create_neo_value(neo_scope current, neo_atom atom);
void free_neo_value(neo_value value);

neo_atom neo_value_get_atom(neo_value self);
void neo_value_set_atom(neo_value self, neo_atom atom);

neo_type neo_value_get_type(neo_value self);
void *neo_value_get_data(neo_value self);

int8_t neo_value_convert(neo_value self, uint32_t type, void *output);

#define neo_value_get_type_name(val)                                           \
  (neo_type_get_name(neo_value_get_type(val)))
#endif