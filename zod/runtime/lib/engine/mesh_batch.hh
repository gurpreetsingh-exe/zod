#include "engine/entity.hh"
#include "gpu/backend.hh"

namespace zod {

class GPUMeshBatch {
public:
  GPUMeshBatch();
  ~GPUMeshBatch() = default;

public:
  auto recompute_batch() -> void;
  auto update_matrix(Entity, const mat4&) -> void;
  auto load_env(Environment&) -> SharedPtr<GPUTexture>;
  auto bind() -> void;
  template <bool Mesh = true>
  constexpr auto batch() const -> GPUBatch& {
    if constexpr (Mesh) {
      return *m_batch;
    } else {
      return *m_cubemap_batch;
    }
  }

private:
  SharedPtr<GPUBatch> m_batch = nullptr;
  SharedPtr<GPUStorageBuffer> m_vertex_buffer = nullptr;
  SharedPtr<GPUStorageBuffer> m_normal_buffer = nullptr;
  SharedPtr<GPUStorageBuffer> m_matrix_buffer = nullptr;
  SharedPtr<GPUBatch> m_cubemap_batch = nullptr;

  std::unordered_map<Entity, usize> m_matrix_offset_map = {};
};

} // namespace zod
