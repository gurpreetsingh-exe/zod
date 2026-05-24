#pragma once

#include "engine/renderer.hh"
#include "sodium/widgets/image.hh"

namespace zod {

class GameViewport : public sodium::Image {
public:
  explicit GameViewport(Renderer&);

  auto render() -> void;
  auto arrange(const Rect&) -> void override;
  auto on_mouse_down(const Event&) -> EventResponse override;
  auto on_mouse_up(const Event&) -> EventResponse override;
  auto on_mouse_move(const Event&) -> EventResponse override;

protected:
  virtual auto on_resize(vec2) -> void {}
  virtual auto on_viewport_event(const Event&) -> EventResponse;

  auto renderer() -> Renderer& { return m_renderer; }
  auto renderer() const -> const Renderer& { return m_renderer; }

private:
  Renderer& m_renderer;
  i32 m_width = 0;
  i32 m_height = 0;
};

} // namespace zod
