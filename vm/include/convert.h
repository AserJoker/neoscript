#ifndef __NEO_VM_CONVERT__
#define __NEO_VM_CONVERT__
#include "engine/include/type.h"
#include "vm.h"
#include <stdint.h>
int8_t neo_convert_to_string(neo_vm vm, neo_value value, cstring *output);
int8_t neo_convert_to_boolean(neo_vm vm, neo_value value, int8_t *output);
#endif