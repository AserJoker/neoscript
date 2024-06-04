#ifndef __NEO_ENGINE_TYPE__
#define __NEO_ENGINE_TYPE__
#include <stddef.h>
#include <stdint.h>
typedef struct _neo_context *neo_context;

typedef struct _neo_type *neo_type;
typedef struct _neo_atom *neo_atom;
typedef struct _neo_value *neo_value;

typedef neo_value (*neo_function)(neo_context ctx, size_t argc,
                                  neo_value *argv);

typedef void (*neo_hook_init_fn)(void *buf, void *input, void *);
typedef void (*neo_hook_dispose_fn)(void *buf, void *);
typedef neo_atom (*neo_hook_copy_fn)(neo_atom atom, void *);
typedef char *(*neo_hook_to_string_fn)(void *data, void *);
typedef int8_t (*neo_hook_to_boolean_fn)(void *data, void *);

typedef struct _neo_type_hook {
  neo_hook_init_fn init;
  void *init_arg;
  neo_hook_dispose_fn dispose;
  void *dispose_arg;
  neo_hook_copy_fn copy;
  void *copy_arg;
} neo_type_hook;

neo_type create_neo_type(const int32_t name, const size_t size,
                         neo_type_hook *hook);
void free_neo_type(neo_type type);

neo_type_hook neo_type_get_hook(neo_type self);
const int32_t neo_type_get_name(neo_type self);
const size_t neo_type_get_size(neo_type self);

#endif