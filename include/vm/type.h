#ifndef __NEO_VM_TYPE__
#define __NEO_VM_TYPE__
#include "engine/runtime.h"
#define NEO_VM_TYPE_EXCEPTION -1
#define NEO_VM_TYPE_NULL NEO_TYPE_NULL
#define NEO_VM_TYPE_INT8 1
#define NEO_VM_TYPE_INT16 2
#define NEO_VM_TYPE_INT32 3
#define NEO_VM_TYPE_INT64 4
#define NEO_VM_TYPE_UINT8 5
#define NEO_VM_TYPE_UINT16 6
#define NEO_VM_TYPE_UINT32 7
#define NEO_VM_TYPE_UINT64 8
#define NEO_VM_TYPE_FLOAT32 9
#define NEO_VM_TYPE_FLOAT64 10
#define NEO_VM_TYPE_BOOLEAN 12
#define NEO_VM_TYPE_STRING 13
#define NEO_VM_TYPE_FUNCTION 14
#define NEO_VM_TYPE_OBJECT 15
#define NEO_VM_TYPE_ARRAY 16

#define CHECK_TYPE(TYPE)                                                       \
  do {                                                                         \
    if (neo_value_get_type_name(value) != TYPE) {                              \
      char buf[1024] = {0};                                                    \
      sprintf(buf, "cannot get %s value from:0x%x", #TYPE,                     \
              neo_value_get_type_name(value));                                 \
      neo_context_throw(                                                       \
          ctx, create_neo_exception(ctx, buf, NULL, __FILE__, __LINE__, 1));   \
    }                                                                          \
  } while (0)
#endif