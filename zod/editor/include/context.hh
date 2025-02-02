#pragma once

#include "application.hh"
#include "asset_manager.hh"
#include "backend.hh"
#include "environment.hh"
#include "event.hh"
#include "font.hh"
#include "node_types.hh"

namespace zod {

class Layout;

class ZCtxt : SApplication {
public:
  ZCtxt(fs::path);
  static auto get() -> ZCtxt&;
  static auto create(fs::path) -> void;
  static auto drop() -> void;
  auto run(fs::path) -> void;
  auto get_normals() -> Shared<GPUStorageBuffer> { return m_ssbo; }
  auto get_batch() -> Shared<GPUBatch> { return m_batch; }
  auto get_node_tree() -> Shared<NodeTree> { return m_node_tree; }
  auto get_asset_manager() -> AssetManager& { return m_asset_manager; }
  auto get_texture() -> Shared<GPUTexture> { return m_texture; }
  auto get_rd_shader() -> Shared<GPUShader> { return m_rd_shader; }
  auto get_env() -> Environment& { return m_env; }

private:
  auto on_event(Event&) -> void;

private:
  Shared<GPUBatch> m_batch;
  Shared<NodeTree> m_node_tree;
  Shared<GPUStorageBuffer> m_ssbo;
  Shared<GPUStorageBuffer> m_vertex_buffer;
  Shared<GPUTexture> m_texture;
  Shared<GPUShader> m_rd_shader;
  AssetManager m_asset_manager;
  Environment m_env;
};

} // namespace zod
