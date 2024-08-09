#pragma once

#include "application/context.hh"
#include "widget.hh"

namespace zod {

struct UIUbo {
  glm::mat4 view_projection_mat;
  f32 width;
  f32 height;
};

extern int border;
extern f32 factor;

class Layout {
public:
  Layout()
      : m_areas(std::vector<Unique<Widget>>()),
        m_uniform_buffer(
            GPUBackend::get().create_uniform_buffer(sizeof(UIUbo))) {}

public:
  auto init() -> void {
    auto uiubo =
        UIUbo { glm::ortho(0.f, 64.0f, 0.f, 64.0f, -1.f, 1.f), 64.0f, 64.0f };
    m_uniform_buffer->upload_data(&uiubo, sizeof(UIUbo));
  }

  auto add_area(Unique<Widget> widget) -> void {
    m_areas.push_back(std::move(widget));
  }

  auto on_event(Event& event) -> void {
    for (const auto& node : m_areas) { node->on_event(event); }
  }

  auto draw() -> void {
    for (const auto& node : m_areas) { node->draw(); }
  }

  auto get_widget(u16 x, u16 y) -> Widget* {
    for (const auto& node : m_areas) {
      if (node->x <= x and node->y <= y and node->x + node->w > x and
          node->y + node->h > y) {
        return node->get_widget(x, y);
      }
    }

    return nullptr;
  }

private:
  std::vector<Unique<Widget>> m_areas;
  Shared<GPUUniformBuffer> m_uniform_buffer;
};

} // namespace zod
