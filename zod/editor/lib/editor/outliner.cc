#include <imgui.h>

#include "editor.hh"
#include "engine/components.hh"
#include "engine/project.hh"
#include "outliner.hh"
#include "theme.hh"

namespace zod {

static auto imgui_color(vec4 color) -> ImVec4 {
  return { color.x, color.y, color.z, color.w };
}

static auto dim(vec4 color, f32 alpha) -> ImVec4 {
  return { color.x, color.y, color.z, alpha };
}

static auto object_type(Entity entity) -> const char* {
  if (entity.has_component<StaticMeshComponent>()) {
    return "StaticMeshComponent";
  }
  if (entity.has_component<LightComponent>()) {
    return "LightComponent";
  }
  if (entity.has_component<CameraComponent>()) {
    return "CameraComponent";
  }
  if (entity.has_component<SkyboxComponent>()) {
    return "SkyboxComponent";
  }
  return "Component";
}

static auto draw_toolbar(auto&& draw_add_menu) -> void {
  ImGui::AlignTextToFramePadding();
  ImGui::TextDisabled("Search Components");
  ImGui::SameLine();
  auto button_width = 88.0f;
  ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - button_width);
  if (ImGui::Button("+ Add", ImVec2(button_width, 0.0f))) {
    ImGui::OpenPopup("Add Menu");
  }
  if (ImGui::BeginPopup("Add Menu")) {
    draw_add_menu();
    ImGui::EndPopup();
  }
  ImGui::Separator();
}

static auto draw_row(Entity entity, bool selected) -> bool {
  auto& theme = Theme::get();
  auto row_height = 26.0f;
  auto left_padding = 10.0f;

  ImGui::TableNextRow(ImGuiTableRowFlags_None, row_height);
  if (selected) {
    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                           ImGui::GetColorU32(imgui_color(theme.highlight)));
  }

  ImGui::TableSetColumnIndex(0);
  auto& name = entity.get_component<IdentifierComponent>().identifier;
  ImGui::PushID(static_cast<int>(std::hash<Entity> {}(entity)));

  auto text_pos = ImGui::GetCursorScreenPos();
  text_pos.x += left_padding;
  text_pos.y +=
      std::max(0.0f, (row_height - ImGui::GetTextLineHeight()) * 0.5f);
  ImGui::Selectable("##row", selected, ImGuiSelectableFlags_SpanAllColumns,
                    ImVec2(0.0f, row_height));
  auto clicked = ImGui::IsItemClicked();
  ImGui::GetWindowDrawList()->AddText(text_pos,
                                      ImGui::GetColorU32(ImGuiCol_Text), name);
  ImGui::PopID();

  ImGui::TableSetColumnIndex(1);
  auto type_pos = ImGui::GetCursorScreenPos();
  type_pos.y +=
      std::max(0.0f, (row_height - ImGui::GetTextLineHeight()) * 0.5f);
  ImGui::Dummy(ImVec2(0.0f, row_height));
  ImGui::GetWindowDrawList()->AddText(
      type_pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), object_type(entity));

  return clicked;
}

auto Outliner::draw_imp(Geometry&) -> void {
  auto& editor = Editor::get();
  auto scene = g_project->active_scene();
  auto draw_add_menu = [&] {
    if (ImGui::MenuItem("Empty Component")) {
      editor.set_active_object(scene->create());
    }
    if (ImGui::MenuItem("Static Mesh Component")) {
      auto cube = scene->create();
      cube.add_component<StaticMeshComponent>(Mesh::cube());
      editor.set_active_object(cube);
    }
    if (ImGui::MenuItem("Light")) {
      auto light = scene->create();
      light.add_component<LightComponent>();
      editor.set_active_object(light);
    }
  };

  auto& theme = Theme::get();
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7.0f, 4.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.0f, 3.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
  ImGui::PushStyleColor(ImGuiCol_Button, imgui_color(theme.foreground));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, imgui_color(theme.highlight));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, imgui_color(theme.highlight));
  ImGui::PushStyleColor(ImGuiCol_Header, imgui_color(theme.highlight));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, dim(theme.highlight, 0.65f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, imgui_color(theme.highlight));
  ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, imgui_color(theme.foreground));
  ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt,
                        ImVec4(1.0f, 1.0f, 1.0f, 0.025f));
  ImGui::PushStyleColor(ImGuiCol_Border, dim(theme.highlight, 0.65f));

  draw_toolbar(draw_add_menu);
  ImGui::Dummy({ 0.0f, 6.0f });

  if (ImGui::BeginPopupContextWindow("Add Context Menu",
                                     ImGuiPopupFlags_MouseButtonRight)) {
    draw_add_menu();
    ImGui::EndPopup();
  }

  auto table_flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV |
                     ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp |
                     ImGuiTableFlags_ScrollY;
  if (ImGui::BeginTable("OutlinerTable", 2, table_flags, ImVec2(0.0f, 0.0f))) {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Component", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 118.0f);
    ImGui::TableHeadersRow();

    auto count = usize(0);
    for (auto& entity_id : (*scene)->view<IdentifierComponent>()) {
      ++count;
      auto entity = Entity(entity_id, scene.get());
      if (draw_row(entity, editor.active_object() == entity)) {
        editor.set_active_object(entity);
      }
    }

    if (count == 0) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Dummy({ 0.0f, 20.0f });
      ImGui::TextDisabled("Right click or + Add to create a component");
    }

    ImGui::EndTable();
  }

  ImGui::PopStyleColor(9);
  ImGui::PopStyleVar(5);
}

auto Outliner::on_event(Event& event) -> void {}

} // namespace zod
