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

typedef void (*neo_hook_init_fn)(void *buf, void *input);
typedef void (*neo_hook_dispose_fn)(void *buf);

typedef struct _neo_type_hook {
  neo_hook_init_fn init;
  neo_hook_dispose_fn dispose;
} neo_type_hook;

neo_type create_neo_type(const int32_t name, const size_t size,
                         neo_type_hook *hook);
void free_neo_type(neo_type type);

neo_type_hook neo_type_get_hook(neo_type self);
const int32_t neo_type_get_name(neo_type self);
const size_t neo_type_get_size(neo_type self);

#endif