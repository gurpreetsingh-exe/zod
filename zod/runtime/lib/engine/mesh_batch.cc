#include "./mesh_batch.hh"
#include "engine/components.hh"
#include "engine/runtime.hh"

namespace zod {

static constexpr usize BUFFER_INIT_SIZE = 8 * 1024 * 1024;
vec2 MEGA_TEXTURE_SIZE = vec2(10 * 1024, 8 * 1024);

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
  u32 roughness_texture_index;
};

struct LightInfo {
  u32 index;
  f32 a;
  f32 b;
};

GPUMeshBatch::GPUMeshBatch()
    : m_vertex_buffer(GPUBackend::get().create_storage_buffer()),
      m_normal_buffer(GPUBackend::get().create_storage_buffer()),
      m_uv_buffer(GPUBackend::get().create_storage_buffer()),
      m_matrix_buffer(GPUBackend::get().create_storage_buffer()),
      m_mesh_info(GPUBackend::get().create_storage_buffer()),
      m_mega_texture(
          GPUBackend::get().create_texture({ .width = i32(MEGA_TEXTURE_SIZE.x),
                                             .height = i32(MEGA_TEXTURE_SIZE.y),
                                             .mips = 8 })),
      m_texture_info(GPUBackend::get().create_storage_buffer()),
      m_light_info(GPUBackend::get().create_storage_buffer()) {
  m_vertex_buffer->upload_data(nullptr, BUFFER_INIT_SIZE);
  m_normal_buffer->upload_data(nullptr, BUFFER_INIT_SIZE);
  m_uv_buffer->upload_data(nullptr, BUFFER_INIT_SIZE);
  m_matrix_buffer->upload_data(nullptr, 64000);
  m_mesh_info->upload_data(nullptr, 500 * sizeof(GPUMeshInfo));
  m_texture_info->upload_data(nullptr, 500 * sizeof(TextureInfo));
  m_light_info->upload_data(nullptr, 500 * sizeof(u32));
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

struct Matrix {
  mat4 model_matrix;
  mat4 inv_model_matrix;
};

auto GPUMeshBatch::recompute_batch() -> void {
  // TODO: make another version of this function which just appends
  // the new mesh at the end instead of recomputing the whole thing
  auto& scene = Runtime::get().scene();
  auto view = scene->view<TransformComponent>();
  auto matrix = Vector<Matrix>();
  matrix.reserve(view.size());
  auto i = usize(0);
  for (auto entity_id : view) {
    auto entity = Entity(entity_id, std::addressof(scene));
    auto mat = *entity.get_component<TransformComponent>();
    m_matrix_offset_map[entity] = matrix.size() * sizeof(Matrix);
    matrix.push_back({ mat, transpose(inverse(mat)) });
    ++i;
  }
  m_matrix_buffer->upload_data(matrix.data(), matrix.size() * sizeof(Matrix));

  auto offset = usize(0);
  auto indices = Vector<u32>();
  auto indirect = Vector<DrawElementsIndirectCommand>();
  auto mesh_info = Vector<GPUMeshInfo>();
  auto lights = Vector<LightInfo>();
  i = usize(0);
  auto matrix_index = usize(0);
  for (auto entity_id : view) {
    auto entity = Entity(entity_id, std::addressof(scene));
    if (entity.has_component<LightComponent>()) {
      auto light = entity.get_component<LightComponent>();
      lights.push_back({ u32(matrix_index), light.a, light.b });
      m_light_offset_map[entity] = { lights.size() * sizeof(LightInfo),
                                     u32(matrix_index) };
      ++matrix_index;
      ++i;
      continue;
    }
    auto mesh = entity.has_component<StaticMeshComponent>()
                    ? entity.get_component<StaticMeshComponent>().mesh
                    : nullptr;
    if (not mesh) {
      ++matrix_index;
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
      auto count = submesh.size;
      auto offset = begin + submesh.offset;
      indirect.emplace_back(count, 1, offset, 0, i);
      mesh_info.emplace_back(matrix_index, submesh.mat.color_texture,
                             submesh.mat.normal_texture,
                             submesh.mat.roughness_texture);
      ++i;
    }
    offset += mesh->points.size();
    ++matrix_index;
  }

  m_mesh_info->update_data(mesh_info.data(),
                           mesh_info.size() * sizeof(GPUMeshInfo));
  LightInfo number_of_lights = { u32(lights.size()), 0.0f, 0.0f };
  m_light_info->update_data(&number_of_lights, sizeof(LightInfo));
  m_light_info->update_data(lights.data(), lights.size() * sizeof(LightInfo),
                            sizeof(LightInfo));
  m_batch = GPUBackend::get().create_batch({}, indices);

  m_batch->upload_indirect(
      indirect.data(), indirect.size() * sizeof(DrawElementsIndirectCommand));
}

auto GPUMeshBatch::update_matrix(Entity entity, const mat4& mat) -> void {
  auto offset = m_matrix_offset_map[entity];
  auto data = Matrix { mat, transpose(inverse(mat)) };
  m_matrix_buffer->update_data(&data, sizeof(Matrix), offset);
}

auto GPUMeshBatch::update_light(Entity entity, const LightComponent& light)
    -> void {
  auto [offset, index] = m_light_offset_map[entity];
  auto light_info = LightInfo { index, light.a, light.b };
  m_light_info->update_data(&light_info, sizeof(LightInfo), offset);
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
  m_light_info->bind(7);
  m_mega_texture->bind();
}

} // namespace zod
