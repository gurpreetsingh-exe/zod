#include <imgui.h>

#include "components.hh"
#include "outliner.hh"
#include "runtime.hh"

namespace zod {

auto Outliner::draw_entity(Entity entity) -> void {
  auto flags = ((m_selection == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
               ImGuiTreeNodeFlags_FramePadding |
               ImGuiTreeNodeFlags_SpanAvailWidth;
  auto& component = entity.get_component<IdentifierComponent>();
  auto open = ImGui::TreeNodeEx(component.identifier, flags, "%s",
                                component.identifier);
  if (ImGui::IsItemClicked()) {
    m_selection = entity;
  }
  if (open) {
    ImGui::TreePop();
  }
}

template <bool First = false>
auto draw_column(const char* text, usize index, bool highlight) -> bool {
  ImGui::TableNextColumn();
  ImGui::TableSetColumnIndex(index);
  if constexpr (First) {
    ImGui::Selectable(text, highlight, ImGuiSelectableFlags_SpanAllColumns);
  } else {
    ImGui::Text(text);
  }
  return ImGui::IsItemClicked();
}

static auto draw_row(const char* name, const char* type, bool selected)
    -> bool {
  auto clicked = false;
  ImGui::TableNextRow();
  if (not selected) {
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                          ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  }
  clicked |= draw_column<true>(name, 0, selected);
  clicked |= draw_column(type, 1, selected);
  if (not selected) {
    ImGui::PopStyleColor();
  }
  return clicked;
}

auto Outliner::draw_imp(Geometry&) -> void {
  auto& scene = Runtime::get().scene();
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
  if (ImGui::BeginPopupContextWindow("Add Menu",
                                     ImGuiPopupFlags_MouseButtonRight)) {
    if (ImGui::MenuItem("Empty")) {
      m_selection = scene.create();
    }

    ImGui::EndPopup();
  }

  auto flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersV;
  if (ImGui::BeginTable("asd", 2, flags)) {
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Type");
    ImGui::TableHeadersRow();
    usize i = 1;
    for (auto& entity_id : scene.m_registry.view<IdentifierComponent>()) {
      ImGui::TableNextRow();
      auto entity = Entity(entity_id, std::addressof(scene));
      auto& component = entity.get_component<IdentifierComponent>();
      if (draw_row(component.identifier, "Object", m_selection == entity)) {
        m_selection = entity;
      }
    }
    ImGui::EndTable();
  }
  ImGui::PopStyleVar();
}

auto Outliner::on_event(Event& event) -> void {}

} // namespace zod
