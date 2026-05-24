#include "editor_viewport.hh"

#include "engine/components.hh"
#include "engine/project.hh"

namespace zod {

EditorViewport::EditorViewport(Renderer& renderer) : GameViewport(renderer) {}

auto EditorViewport::on_resize(vec2) -> void {
  if (auto camera = active_camera()) {
    camera->set_window_position(frame().position);
  }
}

auto EditorViewport::on_viewport_event(const Event& event) -> EventResponse {
  auto camera = active_camera();
  if (not camera) {
    return EventResponse::unhandled();
  }

  switch (event.kind) {
    case Event::MouseDown: {
      auto navigation = Navigation::None;
      if (event.button == MouseButton::Left) {
        auto local = event.mouse - frame().position;
        camera->set_pivot_point(local);
        if (event.alt and event.ctrl) {
          navigation = Navigation::Zoom;
        } else if (event.alt and event.shift) {
          navigation = Navigation::Pan;
        }
      } else if (event.button == MouseButton::Right) {
        navigation = Navigation::Rotate;
      }

      camera->set_navigation(navigation);
      if (navigation == Navigation::None) {
        m_navigating = false;
        return EventResponse::unhandled();
      }

      m_navigating = true;
      return EventResponse::handled().capture_mouse(event.button);
    }
    case Event::MouseUp:
      m_navigating = false;
      camera->set_navigation(Navigation::None);
      return EventResponse::handled();
    case Event::MouseMove: {
      if (not m_navigating) {
        return EventResponse::unhandled();
      }

      camera->update(event);
      return EventResponse::handled();
    }
    default:
      break;
  }

  return EventResponse::unhandled();
}

auto EditorViewport::active_camera() const -> SharedPtr<PerspectiveCamera> {
  if (not g_project) {
    return nullptr;
  }

  auto scene = g_project->active_scene();
  if (not scene) {
    return nullptr;
  }

  auto camera_entity = scene->active_camera();
  if (not camera_entity) {
    return nullptr;
  }

  return camera_entity.get_component<CameraComponent>().camera;
}

} // namespace zod
