#ifndef __NEO_ENGINE_TYPE__
#define __NEO_ENGINE_TYPE__
#include <stddef.h>
#include <stdint.h>
typedef struct _neo_context *neo_context;

typedef struct _neo_type *neo_type;
typedef struct _neo_atom *neo_atom;
typedef struct _neo_value *neo_value;

typedef neo_value (*neo_function)(neo_context ctx, neo_value *args,
                                  size_t argc);

typedef void (*neo_operator_init_fn)(void *buf);
typedef void (*neo_operator_dispose_fn)(void *buf);
typedef void *(*neo_operator_get_fn)(neo_atom atom);
typedef int8_t (*neo_operator_set_fn)(neo_atom atom, void *buf);

typedef struct _neo_type_operator {
  neo_operator_init_fn init;
  neo_operator_dispose_fn dispose;
  neo_operator_get_fn get;
  neo_operator_set_fn set;
} neo_type_operator;

neo_type create_neo_type(const uint32_t name, const size_t size,
                         neo_type_operator *operator);
neo_type_operator neo_type_get_operator(neo_type type);
void free_neo_type(neo_type type);

const uint32_t neo_type_get_name(neo_type self);
const size_t neo_type_get_size(neo_type self);

#endif