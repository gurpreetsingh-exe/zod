#include <imgui.h>

#include "core/components.hh"
#include "editor.hh"
#include "properties.hh"

namespace zod {

Properties::Properties()
    : SPanel("Properties", unique<OrthographicCamera>(64.0f, 64.0f)) {}

template <typename T, typename DrawFunc>
auto draw_component(const std::string& name, Entity entity, DrawFunc draw)
    -> void {
  if (not entity) {
    return;
  }
  if (not entity.has_component<T>()) {
    return;
  }

  auto& component = entity.get_component<T>();
  auto flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
               ImGuiTreeNodeFlags_SpanAvailWidth |
               ImGuiTreeNodeFlags_FramePadding;

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2 { 0.f, 2.f });
  auto open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), flags, "%s",
                                name.c_str());
  ImGui::PopStyleVar();

  if (open) {
    draw(component);
    ImGui::TreePop();
  }

  ImGui::Separator();
}

extern auto DragFloat3(const char*, f32*, f32) -> bool;

auto Properties::update() -> void {
  auto& C = Editor::get();
  auto entity = C.active_object();
  if (not entity) {
    return;
  }

  auto& name = entity.get_component<IdentifierComponent>().identifier;
  ImGui::SeparatorText(name);
  if (ImGui::Button("Add")) {}

  draw_component<TransformComponent>("Transform", entity, [&](auto& component) {
    auto needs_update =
        DragFloat3("Location", (float*)&component.position, 0.1f);
    needs_update |= DragFloat3("Rotation", (float*)&component.rotation, 1.0f);
    needs_update |= DragFloat3("Scale", (float*)&component.scale, 0.1f);
    if (needs_update) {
      C.update_matrix(entity, *component);
    }
  });
}

} // namespace zod
