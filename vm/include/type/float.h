#ifndef __NEO_VM_TYPE_FLOAT__
#define __NEO_VM_TYPE_FLOAT__
#include "runtime.h"
void neo_init_float(neo_runtime runtime);
neo_value create_neo_float(neo_context ctx, double value);
double neo_value_to_float(neo_context ctx, neo_value value);
void neo_float_set_value(neo_context ctx, neo_value value, double val);
#endif