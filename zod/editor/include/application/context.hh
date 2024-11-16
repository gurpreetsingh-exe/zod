#pragma once

#include "application.hh"
#include "asset_manager.hh"
#include "backend.hh"
#include "event.hh"
#include "font.hh"
#include "nodes/node_tree.hh"

namespace zod {

class Layout;

#define GPU_TIME(name, ...) ZCtxt::get().with_scope(name, [&] __VA_ARGS__);

class ZCtxt : SApplication {
public:
  ZCtxt();
  static auto get() -> ZCtxt&;
  static auto create() -> void;
  static auto drop() -> void;
  auto run(fs::path) -> void;
  auto get_normals() -> Shared<GPUStorageBuffer> { return m_ssbo; }
  auto get_batch() -> Shared<GPUBatch> { return m_batch; }
  auto get_node_tree() -> Shared<NodeTree> { return m_node_tree; }

  template <typename Callback>
  auto with_scope(const std::string& name, Callback cb) -> void {
    if (not m_queries.contains(name)) {
      m_queries[name] = GPUBackend::get().create_query();
    }
    auto query = m_queries[name];
    query->begin();
    cb();
    query->end();
    m_times[name] = query->get_time();
  }

  auto get_asset_manager() -> AssetManager& { return m_asset_manager; }

private:
  auto on_event(Event&) -> void;

private:
  Shared<GPUBatch> m_batch;
  Shared<NodeTree> m_node_tree;
  Shared<GPUStorageBuffer> m_ssbo;
  Shared<GPUStorageBuffer> m_vertex_buffer;
  std::unordered_map<std::string, Shared<GPUQuery>> m_queries;
  std::unordered_map<std::string, f32> m_times;
  AssetManager m_asset_manager;
};

} // namespace zod
