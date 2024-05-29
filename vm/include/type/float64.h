#ifndef __NEO_VM_FLOAT64__
#define __NEO_VM_FLOAT64__
#include "runtime.h"
void neo_init_float64(neo_runtime runtime);
neo_value create_neo_float64(neo_context ctx, double value);
double neo_value_to_float64(neo_context ctx, neo_value value);
#endif