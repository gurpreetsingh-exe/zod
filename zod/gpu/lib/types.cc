#include "types.hh"

namespace zod {

auto gpu_sizeof(GPUDataType type) -> usize {
  switch (type) {
    case GPUDataType::Int:
    case GPUDataType::Float:
      return 4;
  }
  UNREACHABLE();
}

} // namespace zod
