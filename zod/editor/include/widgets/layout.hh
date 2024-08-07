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
      : m_areas(std::vector<Unique<Widget>>()), m_corners(std::vector<vec2>()),
        m_uniform_buffer(
            GPUBackend::get().create_uniform_buffer(sizeof(UIUbo))) {}

public:
  auto init() -> void {
    generate();

    f32 position[] = { 0, 1, 1, 1, 0, 0, 1, 0 };
    f32 uv[] = { 0, 1, 1, 1, 0, 0, 1, 0 };
    auto format = std::vector<GPUBufferLayout> {
      { GPUDataType::Float, position, 2, 8 },
      { GPUDataType::Float, uv, 2, 8 },
      { GPUDataType::Float, m_corners.data(), 2, m_corners.size() * 2, true },
    };
    m_corners_batch =
        GPUBackend::get().create_batch(format, { 0, 1, 2, 2, 1, 3 });

    auto indices = std::vector<u32>();
    for (usize i = 0; i < m_corners.size(); i += 4) {
      indices.push_back(i);
      indices.push_back(i + 1);
      indices.push_back(i + 2);
      indices.push_back(i + 2);
      indices.push_back(i + 1);
      indices.push_back(i + 3);
    }

    m_rects_batch = GPUBackend::get().create_batch(
        { { GPUDataType::Float, m_corners.data(), 2, m_corners.size() * 2 } },
        indices);

    auto uiubo =
        UIUbo { glm::ortho(0.f, 64.0f, 0.f, 64.0f, -1.f, 1.f), 64.0f, 64.0f };
    m_uniform_buffer->upload_data(&uiubo, sizeof(UIUbo));

    m_rect = GPUBackend::get().create_batch(
        { { GPUDataType::Float, position, 2, 8 } }, { 0, 1, 2, 2, 1, 3 });
  }

  auto add_area(Unique<Widget> widget) -> void {
    m_areas.push_back(std::move(widget));
  }

  auto calculate(f32 x, f32 y, f32 w, f32 h) -> void {
    for (const auto& node : m_areas) { node->calculate(x, y, w, h); }
  }

  auto generate() -> void {
    for (const auto& node : m_areas) { node->generate(m_corners); }
  }

  auto update(f32 x, f32 y, f32 w, f32 h) -> void {
    auto [w1, h1] = ZCtxt::get().get_window_size();
    auto uiubo =
        // UIUbo { glm::ortho(0.f, f32(w), 0.f, f32(h), -1.f, 1.f), w, h };
        UIUbo { glm::ortho(0.f, f32(w1), 0.f, f32(h1), -1.f, 1.f), w, h };
    // fmt::println("{}", 2.0f / uiubo.view_projection_mat[0][0]);
    // fmt::println("{}", 2.0f / uiubo.view_projection_mat[1][1]);
    m_uniform_buffer->upload_data(&uiubo, sizeof(UIUbo));

    m_corners.clear();
    calculate(x, y, w, h);
    generate();
    m_corners_batch->update_binding(2, m_corners.data(),
                                    m_corners.size() * sizeof(vec2));
    m_rects_batch->update_binding(0, m_corners.data(),
                                  m_corners.size() * sizeof(vec2));
  }

  auto on_event(Event& event) -> void {
    if (event.kind == Event::WindowResize) {
      auto w = event.size[0];
      auto h = event.size[1];
      // f32 b = glm::round(border * factor);
      // f32 pw = w - b * 2;
      // f32 ph = h - b * 2;
      // update(b, b, pw, ph);
      update(0, 0, w, h);
    }
    for (const auto& node : m_areas) { node->on_event(event); }
  }

  auto draw(Shared<GPUShader> s1, Shared<GPUShader> s2, int border,
            glm::vec3 surface0) -> void {
    m_uniform_buffer->bind();
    s1->bind();
    s1->uniform("u_border", border);
    s1->uniform("u_color", surface0);
    // m_rects_batch->draw(s1);
    DrawData draw_data = { m_rect, s1 };
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    for (const auto& node : m_areas) { node->draw(draw_data); }
    // m_uniform_buffer->bind();
    // s2->bind();
    // s2->uniform("u_border", border);
    // s2->uniform("u_color", surface0);
    // m_corners_batch->draw_instanced(s2, m_corners.size());
    glDisable(GL_BLEND);
  }

  auto get_widget(f32 x, f32 y) -> Widget* {
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
  std::vector<vec2> m_corners;
  Shared<GPUBatch> m_corners_batch;
  Shared<GPUBatch> m_rects_batch;
  Shared<GPUUniformBuffer> m_uniform_buffer;
  Shared<GPUBatch> m_rect;
};

} // namespace zod
