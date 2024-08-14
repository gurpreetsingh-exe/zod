#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "backend.hh"
#include "base/math.hh"
#include "window.hh"

#include "imgui_layer.hh"

namespace zod {

class Layout;

#define GPU_TIME(name, ...) ZCtxt::get().with_scope(name, [&] __VA_ARGS__);

class ZCtxt {
public:
  ZCtxt();
  static auto get() -> ZCtxt&;
  static auto create() -> void;
  static auto drop() -> void;
  auto run(fs::path) -> void;
  auto get_window_size() -> std::tuple<i32, i32> {
    return m_window->get_size();
  }
  auto get_window() -> Window& { return *m_window; }

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

private:
  auto on_event(Event&) -> void;

private:
  Unique<Window> m_window;
  Shared<GPUBatch> m_batch;
  Shared<GPUStorageBuffer> m_ssbo;
  Shared<GPUFrameBuffer> m_framebuffer;
  Unique<ImGuiLayer> m_imgui_layer;
  std::unordered_map<std::string, Shared<GPUQuery>> m_queries;
  std::unordered_map<std::string, f32> m_times;
};

} // namespace zod
