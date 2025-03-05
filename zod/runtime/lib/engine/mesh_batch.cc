#include "./mesh_batch.hh"
#include "engine/components.hh"
#include "engine/runtime.hh"

namespace zod {

static constexpr usize BUFFER_INIT_SIZE = 1024 * 1024;

struct DrawElementsIndirectCommand {
  u32 count;
  u32 instanceCount;
  u32 firstIndex;
  u32 baseVertex;
  u32 baseInstance;
};

GPUMeshBatch::GPUMeshBatch()
    : m_vertex_buffer(GPUBackend::get().create_storage_buffer()),
      m_normal_buffer(GPUBackend::get().create_storage_buffer()),
      m_matrix_buffer(GPUBackend::get().create_storage_buffer()) {
  m_vertex_buffer->upload_data(nullptr, BUFFER_INIT_SIZE);
  m_normal_buffer->upload_data(nullptr, BUFFER_INIT_SIZE);
  m_matrix_buffer->upload_data(nullptr, BUFFER_INIT_SIZE);
  m_batch = GPUBackend::get().create_batch({});
  m_batch->upload_indirect(nullptr, 0);

  f32 position[] = {
    -1, -1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1,
    -1, -1, 1,  1, -1, 1,  1, 1, 1,  -1, 1, 1,
  };

  auto format = Vector<GPUBufferLayout> {
    { GPUDataType::Float, position, 3, 24 },
  };
  m_cubemap_batch = GPUBackend::get().create_batch(
      format, { 1, 2, 0, 2, 3, 0, 6, 2, 1, 1, 5, 6, 6, 5, 4, 4, 7, 6,
                6, 3, 2, 7, 3, 6, 3, 7, 0, 7, 4, 0, 5, 1, 0, 4, 5, 0 });
}

auto GPUMeshBatch::recompute_batch() -> void {
  // TODO: make another version of this function which just appends
  // the new mesh at the end instead of recomputing the whole thing
  auto& scene = Runtime::get().scene();
  auto view = scene->view<TransformComponent>();
  auto* matrix = new f32[view.size() * 16];
  auto i = usize(0);
  auto buffer_size = usize(0);
  for (auto entity_id : view) {
    auto entity = Entity(entity_id, std::addressof(scene));
    auto mesh = entity.has_component<StaticMeshComponent>()
                    ? entity.get_component<StaticMeshComponent>().mesh
                    : nullptr;
    auto mat = *entity.get_component<TransformComponent>();
    std::memcpy(&matrix[i * 16], ADDROF(mat), sizeof(mat4));
    buffer_size += mesh ? mesh->points.size() * sizeof(vec3) : 0;
    m_matrix_offset_map[entity] = i * sizeof(mat4);
    ++i;
  }
  m_matrix_buffer->upload_data(matrix, sizeof(mat4) * i);
  delete[] matrix;

  auto offset_v = usize(0);
  auto offset_n = usize(0);
  auto indices = Vector<u32>();
  auto indirect = Vector<DrawElementsIndirectCommand>();
  i = usize(0);
  for (auto entity_id : view) {
    auto entity = Entity(entity_id, std::addressof(scene));
    auto mesh = entity.has_component<StaticMeshComponent>()
                    ? entity.get_component<StaticMeshComponent>().mesh
                    : nullptr;
    if (not mesh) {
      indirect.emplace_back(0, 1, 0, 0, i);
      ++i;
      continue;
    }
    m_vertex_buffer->update_data(mesh->points.data(),
                                 mesh->points.size() * sizeof(vec3),
                                 offset_v * sizeof(vec3));
    m_normal_buffer->update_data(mesh->normals.data(),
                                 mesh->normals.size() * sizeof(vec3),
                                 offset_n * sizeof(vec3));
    auto begin = indices.size();
    for (const auto& prim : mesh->prims) {
      for (auto n : prim.points) { indices.push_back(n + offset_v); }
    }
    indirect.emplace_back(indices.size() - begin, 1, begin, 0, i);
    offset_v += mesh->points.size();
    offset_n += mesh->normals.size();
    ++i;
  }
  m_batch = GPUBackend::get().create_batch({}, indices);

  m_batch->upload_indirect(
      indirect.data(), indirect.size() * sizeof(DrawElementsIndirectCommand));
}

auto GPUMeshBatch::update_matrix(Entity entity, const mat4& mat) -> void {
  auto offset = m_matrix_offset_map[entity];
  m_matrix_buffer->update_data(ADDROF(mat), sizeof(mat4), offset);
}

auto GPUMeshBatch::load_env(Environment& env)
    -> SharedPtr<GPUTexture> {
  const auto path = fs::path(env.hdr.s);
  return fs::exists(path) ? GPUBackend::get().create_texture(
                                GPUTextureType::TextureCube, path)
                          : nullptr;
}

auto GPUMeshBatch::bind() -> void {
  m_normal_buffer->bind(0);
  m_matrix_buffer->bind(1);
  m_vertex_buffer->bind(2);
}

} // namespace zod
