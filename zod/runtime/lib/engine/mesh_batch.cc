#include "./mesh_batch.hh"
#include "engine/components.hh"
#include "engine/runtime.hh"

namespace zod {

static constexpr usize BUFFER_INIT_SIZE = 8 * 1024 * 1024;

struct DrawElementsIndirectCommand {
  u32 count;
  u32 instanceCount;
  u32 firstIndex;
  u32 baseVertex;
  u32 baseInstance;
};

struct GPUMeshInfo {
  u32 matrix_index;
  u32 base_color_texture_index;
  u32 normal_texture_index;
};

GPUMeshBatch::GPUMeshBatch()
    : m_vertex_buffer(GPUBackend::get().create_storage_buffer()),
      m_normal_buffer(GPUBackend::get().create_storage_buffer()),
      m_uv_buffer(GPUBackend::get().create_storage_buffer()),
      m_matrix_buffer(GPUBackend::get().create_storage_buffer()),
      m_mesh_info(GPUBackend::get().create_storage_buffer()),
      m_mega_texture(GPUBackend::get().create_texture(
          { .width = 12 * 1024, .height = 8 * 1024, .mips = 8 })),
      m_texture_info(GPUBackend::get().create_storage_buffer()) {
  m_vertex_buffer->upload_data(nullptr, BUFFER_INIT_SIZE);
  m_normal_buffer->upload_data(nullptr, BUFFER_INIT_SIZE);
  m_uv_buffer->upload_data(nullptr, BUFFER_INIT_SIZE);
  m_matrix_buffer->upload_data(nullptr, 64000);
  m_mesh_info->upload_data(nullptr, 500 * sizeof(GPUMeshInfo));
  m_texture_info->upload_data(nullptr, 500 * sizeof(TextureInfo));
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
  for (auto entity_id : view) {
    auto entity = Entity(entity_id, std::addressof(scene));
    auto mat = *entity.get_component<TransformComponent>();
    std::memcpy(&matrix[i * 16], ADDROF(mat), sizeof(mat4));
    m_matrix_offset_map[entity] = i * sizeof(mat4);
    ++i;
  }
  m_matrix_buffer->upload_data(matrix, sizeof(mat4) * i);
  delete[] matrix;

  auto offset = usize(0);
  auto indices = Vector<u32>();
  auto indirect = Vector<DrawElementsIndirectCommand>();
  auto mesh_info = Vector<GPUMeshInfo>();
  i = usize(0);
  auto matrix_index = usize(0);
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
    m_vertex_buffer->update_data(mesh->points.data(), mesh->points.size_bytes(),
                                 offset * sizeof(vec3));
    m_normal_buffer->update_data(mesh->normals.data(),
                                 mesh->normals.size_bytes(),
                                 offset * sizeof(vec3));
    m_uv_buffer->update_data(mesh->uvs.data(), mesh->uvs.size_bytes(),
                             offset * sizeof(vec2));
    auto begin = indices.size();
    for (const auto& prim : mesh->prims) {
      for (auto n : prim.points) { indices.push_back(n + offset); }
    }
    for (auto submesh : mesh->submeshes) {
      // auto count = indices.size() - begin;
      auto count = submesh.size;
      auto offset = begin + submesh.offset;
      indirect.emplace_back(count, 1, offset, 0, i);
      mesh_info.emplace_back(matrix_index, submesh.mat.color_texture,
                             submesh.mat.normal_texture);
      ++i;
    }
    offset += mesh->points.size();
    ++matrix_index;
  }

  m_mesh_info->update_data(mesh_info.data(),
                           mesh_info.size() * sizeof(GPUMeshInfo));

  m_batch = GPUBackend::get().create_batch({}, indices);

  m_batch->upload_indirect(
      indirect.data(), indirect.size() * sizeof(DrawElementsIndirectCommand));
}

auto GPUMeshBatch::update_matrix(Entity entity, const mat4& mat) -> void {
  auto offset = m_matrix_offset_map[entity];
  m_matrix_buffer->update_data(ADDROF(mat), sizeof(mat4), offset);
}

auto GPUMeshBatch::load_env(Environment& env) -> SharedPtr<GPUTexture> {
  const auto path = fs::path(env.hdr.s);
  return fs::exists(path)
             ? GPUBackend::get().create_texture(
                   { .type = GPUTextureType::TextureCube, .path = path })
             : nullptr;
}

auto GPUMeshBatch::bind() -> void {
  m_normal_buffer->bind(0);
  m_matrix_buffer->bind(1);
  m_vertex_buffer->bind(2);
  m_mesh_info->bind(3);
  m_texture_info->bind(4);
  m_uv_buffer->bind(5);
  m_mega_texture->bind();
}

} // namespace zod
