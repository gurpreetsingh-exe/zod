#pragma once

#include "engine/game_viewport.hh"

namespace zod {

class EditorViewport : public GameViewport {
public:
  explicit EditorViewport(Renderer&);

private:
  auto on_resize(vec2) -> void override;
  auto on_viewport_event(const Event&) -> EventResponse override;
  auto active_camera() const -> SharedPtr<PerspectiveCamera>;
  auto local_event(const Event&) const -> Event;

private:
  bool m_navigating = false;
};

} // namespace zod
