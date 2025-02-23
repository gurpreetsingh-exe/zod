#include "./batch.hh"
#include "./buffer.hh"

namespace zod {

VKBatch::VKBatch(const Vector<GPUBufferLayout>& layouts,
                 const Vector<u32>& indices) {
  m_elements =
      indices.size() ? indices.size() : layouts[0].length / layouts[0].size;
  TODO();
}

} // namespace zod
