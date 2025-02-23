#pragma once

#include "application/application.hh"
#include "application/event.hh"
#include "asset_manager.hh"
#include "engine/entity.hh"
#include "engine/environment.hh"
#include "engine/node_types.hh"
#include "gpu/backend.hh"
#include "imgui_layer.hh"
#include "widgets/layout.hh"

namespace zod {

class Layout;

class Editor : public ILayer {
public:
  Editor();
  static auto get() -> Editor&;
  auto get_normals() -> SharedPtr<GPUStorageBuffer> { return m_ssbo; }
  auto get_batch() -> SharedPtr<GPUBatch> { return m_batch; }
  auto get_node_tree() -> SharedPtr<NodeTree> { return m_node_tree; }
  auto get_asset_manager() -> AssetManager& { return m_asset_manager; }
  auto get_texture() -> SharedPtr<GPUTexture> { return m_texture; }
  auto get_rd_shader() -> SharedPtr<GPUShader> { return m_rd_shader; }
  auto get_env() -> Environment& { return m_env; }
  auto active_object() const -> Entity { return m_active_object; }
  auto set_active_object(Entity e) -> void { m_active_object = e; }
  auto mesh() const -> Mesh* { return m_mesh; }
  auto recompute_batch() -> void;
  auto update_matrix(Entity, const mat4&) -> void;

private:
  auto setup() -> void override;
  auto on_event(Event&) -> void override;
  auto update() -> void override;

private:
  UniquePtr<ImGuiLayer> m_imgui_layer;
  UniquePtr<Layout> m_layout;
  SharedPtr<GPUBatch> m_batch;
  SharedPtr<NodeTree> m_node_tree;
  SharedPtr<GPUStorageBuffer> m_ssbo;
  SharedPtr<GPUStorageBuffer> m_vertex_buffer;
  SharedPtr<GPUStorageBuffer> m_vertex_buffer_out;
  SharedPtr<GPUTexture> m_texture;
  SharedPtr<GPUShader> m_rd_shader;
  AssetManager m_asset_manager;
  Entity m_active_object;
  Environment m_env;

  SharedPtr<GPUStorageBuffer> m_matrix_buffer;
  Mesh* m_mesh;
  std::unordered_map<Entity, usize> m_matrix_offset_map = {};
};

} // namespace zod
