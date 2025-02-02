#include "environment_light.hh"
#include "context.hh"
#include <imgui.h>

namespace zod {

auto EnvironmentLight::draw_imp(Geometry&) -> void {
  auto& env = ZCtxt::get().get_env();
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
