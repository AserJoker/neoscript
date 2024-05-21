#ifndef __NEO_VM_OPERATOR_CONVERT__
#define __NEO_VM_OPERATOR_CONVERT__

#include "engine/type.h"
neo_value neo_vm_opt_convert(neo_context ctx, uint32_t opt, int argc,
                             neo_value *argv);
#endif