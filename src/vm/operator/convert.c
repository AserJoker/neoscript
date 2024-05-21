#include "vm/operator/convert.h"
#include "engine/type.h"

neo_value neo_vm_opt_convert(neo_context ctx, uint32_t opt, int argc,
                             neo_value *argv) {
  neo_value source = argv[0];
  return source;
}