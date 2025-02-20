#include <imgui.h>

#include "context.hh"
#include "core/components.hh"
#include "core/runtime.hh"
#include "outliner.hh"

namespace zod {

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
  auto& C = ZCtxt::get();
  auto& scene = Runtime::get().scene();
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 4.0f));
  if (ImGui::BeginPopupContextWindow("Add Menu",
                                     ImGuiPopupFlags_MouseButtonRight)) {
    if (ImGui::MenuItem("Empty")) {
      C.set_active_object(scene.create());
    }
    if (ImGui::MenuItem("Cube")) {
      auto cube = scene.create();
      cube.add_component<StaticMeshComponent>(C.mesh());
      C.set_active_object(cube);
      C.recompute_batch();
    }

    ImGui::EndPopup();
  }
  ImGui::PopStyleVar();

  if (ImGui::BeginTable("OutlinerTable", 2,
                        ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersV)) {
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Type");
    ImGui::TableHeadersRow();
    usize i = 1;
    for (auto& entity_id : scene.m_registry.view<IdentifierComponent>()) {
      ImGui::TableNextRow();
      auto entity = Entity(entity_id, std::addressof(scene));
      auto& component = entity.get_component<IdentifierComponent>();
      if (draw_row(component.identifier, "Object",
                   C.active_object() == entity)) {
        C.set_active_object(entity);
      }
    }
    ImGui::EndTable();
  }
}

auto Outliner::on_event(Event& event) -> void {}

} // namespace zod
