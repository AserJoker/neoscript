#ifndef __NEO_VM_OPERATOR_ADD__
#define __NEO_VM_OPERATOR_ADD__

#include "engine/type.h"
neo_value neo_vm_opt_add(neo_context ctx, uint32_t opt, int argc,
                         neo_value *argv);
#endif