#ifndef __NEO_VM_OPERATOR_ADD__
#define __NEO_VM_OPERATOR_ADD__
#include "type.h"
neo_value neo_operator_expr(neo_context ctx, uint32_t opt, int argc,
                           neo_value *argv);
#endif