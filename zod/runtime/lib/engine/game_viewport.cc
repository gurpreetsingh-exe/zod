#include "engine/game_viewport.hh"

namespace zod {

GameViewport::GameViewport(Renderer& renderer)
    : sodium::Image(renderer.get_render_target()->get_slot(0)),
      m_renderer(renderer) {}

auto GameViewport::render() -> void { m_renderer.tick(); }

auto GameViewport::arrange(const Rect& bounds) -> void {
  Image::arrange(bounds);

  auto width = std::max(1, i32(bounds.size.x));
  auto height = std::max(1, i32(bounds.size.y));
  if (width == m_width and height == m_height) {
    return;
  }

  m_width = width;
  m_height = height;
  m_renderer.resize(f32(m_width), f32(m_height));
  on_resize({ f32(m_width), f32(m_height) });
}

auto GameViewport::on_mouse_down(const Event& event) -> EventResponse {
  return on_viewport_event(event);
}

auto GameViewport::on_mouse_up(const Event& event) -> EventResponse {
  return on_viewport_event(event);
}

auto GameViewport::on_mouse_move(const Event& event) -> EventResponse {
  return on_viewport_event(event);
}

auto GameViewport::on_viewport_event(const Event&) -> EventResponse {
  return EventResponse::unhandled();
}

} // namespace zod
