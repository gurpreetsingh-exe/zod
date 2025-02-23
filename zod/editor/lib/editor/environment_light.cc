#include <imgui.h>

#include "editor.hh"
#include "engine/camera.hh"
#include "environment_light.hh"

namespace zod {

EnvironmentLight::EnvironmentLight()
    : SPanel("Environment Light", unique<OrthographicCamera>(64.0f, 64.0f)) {}

auto EnvironmentLight::draw_imp(Geometry&) -> void {
  auto& env = Editor::get().get_env();
  ImGui::Combo("Mode", (int*)&env.mode, "Solid Color\0Texture\0\0");
  switch (env.mode) {
    case LightingMode::SolidColor: {
      draw_property(env.color);
    } break;
    case LightingMode::Texture: {
      draw_property(env.hdr);
    } break;
  }
}

auto EnvironmentLight::on_event(Event& event) -> void {}

} // namespace zod
