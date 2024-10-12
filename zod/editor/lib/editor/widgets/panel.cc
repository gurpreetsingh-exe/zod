#include "widgets/panel.hh"

namespace zod {

Panel::Panel(std::string name, Unique<ICamera> camera, bool padding)
    : Widget(std::move(name)), m_camera(std::move(camera)), m_padding(padding),
      m_uniform_buffer(GPUBackend::get().create_uniform_buffer(
          sizeof(CameraUniformBufferStorage))),
      m_framebuffer(GPUBackend::get().create_framebuffer(64.0f, 64.0f)) {}

auto Panel::on_event(Event& event) -> void {
  switch (event.kind) {
    case Event::MouseDown: {
      auto nav = Navigation::None;
      using btn = Event::ButtonKind;
      if (event.button == btn::MouseButtonLeft) {
        m_camera->set_pivot_at_mouse();
        auto any_alt = any_key(Key::LeftAlt, Key::RightAlt);
        if (any_alt and any_key(Key::LeftCtrl, Key::RightCtrl)) {
          nav = Navigation::Zoom;
        } else if (any_alt and any_key(Key::LeftShift, Key::RightShift)) {
          nav = Navigation::Pan;
        }
      } else if (event.button == btn::MouseButtonRight) {
        nav = Navigation::Rotate;
      }
      m_camera->set_navigation(nav);
    } break;

    case Event::MouseUp: {
      m_camera->set_navigation(Navigation::None);
    } break;
  }
}

auto Panel::draw(Geometry& g) -> void {
  if (not m_padding) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  }
  ImGui::Begin(name.c_str());
  m_active = ImGui::IsWindowHovered();
  draw_imp(g);
  ImGui::End();
  if (not m_padding) {
    ImGui::PopStyleVar();
  }
}

} // namespace zod
