#pragma once

#include "widget.hh"

namespace zod {

extern int border;
extern int padding;
extern f32 factor;

class Panel : public Widget {
public:
  Panel() = default;
  auto calculate(f32 x, f32 y, f32 w, f32 h) -> void override {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    // fmt::println("{} {} {} {}", x, y, w, h);
    // this->h = h - 25;
  }

  auto on_event(Event&) -> void override {}

  auto draw(DrawData& data) -> void {
    auto shader = data.shader;
    auto batch = data.batch;
    shader->bind();

    // shader->uniform("u_x", x);
    // shader->uniform("u_y", h - y - 5);
    // shader->uniform("u_width", 100);
    // shader->uniform("u_height", 30);
    // shader->uniform("u_border", border);
    // shader->uniform("u_color", glm::vec3( 0.15f, 0.16f, 0.17f ));
    // batch->draw(shader);

    shader->uniform("u_x", x);
    shader->uniform("u_y", y);
    shader->uniform("u_width", w);
    shader->uniform("u_height", h);
    shader->uniform("u_border", border);
    shader->uniform("u_padding", padding);
    shader->uniform("u_color", glm::vec3( 0.15f, 0.16f, 0.17f ));
    batch->draw(shader);
  }

  auto generate(std::vector<vec2>& offsets) -> void override {
    // offsets.push_back({ x, h - y - 100 });
    // offsets.push_back({ x + 150, h - y - 100 });
    // offsets.push_back({ x, h - y + 28 });
    // offsets.push_back({ x + 150, h - y + 28 });

    // offsets.push_back({ x, y });
    // offsets.push_back({ x + w, y });
    // offsets.push_back({ x, y + h });
    // offsets.push_back({ x + w, y + h });
  }

private:
  static Shared<GPUBatch> m_batch;
};

} // namespace zod
