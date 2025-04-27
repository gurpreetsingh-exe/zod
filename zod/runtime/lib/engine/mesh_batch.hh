#include "engine/entity.hh"
#include "gpu/backend.hh"

namespace zod {

extern vec2 MEGA_TEXTURE_SIZE;

class GPUMeshBatch {
public:
  GPUMeshBatch();
  ~GPUMeshBatch() = default;

public:
  auto recompute_batch() -> void;
  auto update_matrix(Entity, const mat4&) -> void;
  auto load_env(Environment&) -> SharedPtr<GPUTexture>;
  auto mega_texture() const -> SharedPtr<GPUTexture> { return m_mega_texture; }
  auto texture_info() const -> SharedPtr<GPUStorageBuffer> {
    return m_texture_info;
  }
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
  SharedPtr<GPUStorageBuffer> m_uv_buffer = nullptr;
  SharedPtr<GPUStorageBuffer> m_matrix_buffer = nullptr;
  SharedPtr<GPUStorageBuffer> m_mesh_info = nullptr;
  SharedPtr<GPUTexture> m_mega_texture = nullptr;
  SharedPtr<GPUStorageBuffer> m_texture_info = nullptr;
  SharedPtr<GPUStorageBuffer> m_light_indices = nullptr;
  SharedPtr<GPUBatch> m_cubemap_batch = nullptr;

  std::unordered_map<Entity, usize> m_matrix_offset_map = {};
};

} // namespace zod
