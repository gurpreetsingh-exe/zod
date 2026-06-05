#include <imgui.h>

#include "application/platform.hh"
#include "editor.hh"
#include "engine/components.hh"
#include "properties.hh"
#include "theme.hh"

namespace zod {

static constexpr auto DetailsLabelWidth = 92.0f;
static constexpr auto SectionSpacing = 8.0f;

Properties::Properties()
    : SPanel("Properties", shared<OrthographicCamera>(64.0f, 64.0f)) {}

static auto imgui_color(vec4 color) -> ImVec4 {
  return { color.x, color.y, color.z, color.w };
}

static auto dim(vec4 color, f32 alpha) -> ImVec4 {
  return { color.x, color.y, color.z, alpha };
}

static auto draw_empty_state(const char* text) -> void {
  auto avail = ImGui::GetContentRegionAvail();
  ImGui::Dummy({ 0.0f, std::max(24.0f, avail.y * 0.35f) });
  auto text_size = ImGui::CalcTextSize(text);
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                       std::max(0.0f, (avail.x - text_size.x) * 0.5f));
  ImGui::TextDisabled("%s", text);
}

static auto begin_row(const char* label) -> void {
  ImGui::PushID(label);
  ImGui::BeginGroup();
  ImGui::AlignTextToFramePadding();
  ImGui::TextDisabled("%s", label);
  ImGui::SameLine(DetailsLabelWidth);
  ImGui::SetNextItemWidth(-1.0f);
}

static auto end_row() -> void {
  ImGui::EndGroup();
  ImGui::PopID();
}

static auto draw_string_row(const char* label, char* value) -> bool {
  begin_row(label);
  auto changed = ImGui::InputText("##value", value, STRING_PROP_MAX_SIZE);
  end_row();
  return changed;
}

static auto draw_filepath_row(const char* label, char* value) -> bool {
  begin_row(label);
  auto browse_width = 34.0f;
  auto spacing = ImGui::GetStyle().ItemInnerSpacing.x;
  ImGui::SetNextItemWidth(
      std::max(40.0f, ImGui::CalcItemWidth() - browse_width - spacing));
  auto changed = ImGui::InputText("##value", value, STRING_PROP_MAX_SIZE);
  ImGui::SameLine();
  if (ImGui::Button("...", ImVec2(browse_width, 0.0f))) {
    auto path = open_dialog();
    ZASSERT(path.size() < STRING_PROP_MAX_SIZE);
    std::memset(value, 0, STRING_PROP_MAX_SIZE);
    std::memcpy(value, path.c_str(), path.size());
    changed = true;
  }
  end_row();
  return changed;
}

static auto draw_color_row(const char* label, vec3& value) -> bool {
  begin_row(label);
  auto changed = ImGui::ColorEdit3("##value", &value[0],
                                   ImGuiColorEditFlags_Float |
                                       ImGuiColorEditFlags_NoLabel);
  end_row();
  return changed;
}

static auto draw_property_row(Property& prop) -> bool {
  auto changed = false;
  switch (prop.type) {
    case PROP_STRING:
      changed = prop.subtype == PROP_SUBTYPE_FILEPATH
                    ? draw_filepath_row(prop.name, prop.s)
                    : draw_string_row(prop.name, prop.s);
      break;
    case PROP_INT:
      begin_row(prop.name);
      changed = ImGui::DragInt("##value", &prop.i);
      end_row();
      break;
    case PROP_FLOAT:
      begin_row(prop.name);
      changed = ImGui::DragFloat("##value", &prop.f);
      end_row();
      break;
    case PROP_VEC3:
      if (prop.subtype == PROP_SUBTYPE_COLOR) {
        changed = draw_color_row(prop.name, prop.v3);
      } else {
        begin_row(prop.name);
        changed = ImGui::DragFloat3("##value", &prop.v3[0], 0.1f);
        end_row();
      }
      break;
  }
  prop.needs_update = changed;
  return changed;
}

static auto draw_combo_row(const char* label, int* value, const char* items)
    -> bool {
  begin_row(label);
  auto changed = ImGui::Combo("##value", value, items);
  end_row();
  return changed;
}

static auto draw_float_row(const char* label, f32* value) -> bool {
  begin_row(label);
  auto changed = ImGui::DragFloat("##value", value);
  end_row();
  return changed;
}

static auto draw_vec3_row(const char* label, vec3& value, f32 speed) -> bool {
  begin_row(label);
  auto available = ImGui::CalcItemWidth();
  auto gap = ImGui::GetStyle().ItemInnerSpacing.x;
  auto label_size = 18.0f;
  auto field_width =
      std::max(24.0f, (available - gap * 2.0f - label_size * 3.0f) / 3.0f);
  auto changed = false;
  ImGui::PushID(label);
  auto draw_axis = [&](const char* axis, const char* id, f32* axis_value,
                       ImVec4 color) {
    ImGui::TextColored(color, "%s", axis);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(field_width);
    changed |= ImGui::DragFloat(id, axis_value, speed);
  };
  draw_axis("X", "##X", &value.x, { 0.82f, 0.18f, 0.16f, 1.0f });
  ImGui::SameLine();
  draw_axis("Y", "##Y", &value.y, { 0.20f, 0.70f, 0.25f, 1.0f });
  ImGui::SameLine();
  draw_axis("Z", "##Z", &value.z, { 0.20f, 0.42f, 0.90f, 1.0f });
  ImGui::PopID();
  end_row();
  return changed;
}

static auto section_header(const char* name) -> bool {
  auto& theme = Theme::get();
  auto flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
               ImGuiTreeNodeFlags_SpanAvailWidth |
               ImGuiTreeNodeFlags_FramePadding;
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 6.0f));
  ImGui::PushStyleColor(ImGuiCol_Header, imgui_color(theme.foreground));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, imgui_color(theme.highlight));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, imgui_color(theme.highlight));
  auto open = ImGui::TreeNodeEx(name, flags);
  ImGui::PopStyleColor(3);
  ImGui::PopStyleVar();
  return open;
}

template <typename T, typename DrawFunc>
static auto draw_component(const char* name, Entity entity, DrawFunc draw)
    -> void {
  if (not entity or not entity.has_component<T>()) {
    return;
  }

  ImGui::PushID(name);
  if (section_header(name)) {
    auto& component = entity.get_component<T>();
    ImGui::Spacing();
    ImGui::Indent(6.0f);
    if (draw(component)) {
      entity.update_internal<T>();
    }
    ImGui::Unindent(6.0f);
    ImGui::Spacing();
    ImGui::TreePop();
  }
  ImGui::PopID();
  ImGui::Dummy({ 0.0f, SectionSpacing });
}

template <typename T>
static auto draw_add_component(Entity entity, const char* name) -> void {
  if (not entity.has_component<T>() and ImGui::MenuItem(name)) {
    entity.add_component<T>();
    ImGui::CloseCurrentPopup();
  }
}

auto Properties::update() -> void {
  auto& editor = Editor::get();
  auto entity = editor.active_object();
  auto& theme = Theme::get();

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7.0f, 4.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 6.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
  ImGui::PushStyleColor(ImGuiCol_Button, imgui_color(theme.foreground));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, imgui_color(theme.highlight));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, imgui_color(theme.highlight));
  ImGui::PushStyleColor(ImGuiCol_ChildBg, imgui_color(theme.foreground));
  ImGui::PushStyleColor(ImGuiCol_Border, dim(theme.highlight, 0.75f));

  if (not entity) {
    draw_empty_state("No object selected");
    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(5);
    return;
  }

  auto& name = entity.get_component<IdentifierComponent>().identifier;
  auto& uuid = entity.get_component<UUIDComponent>().id;

  ImGui::BeginChild("DetailsHeader", ImVec2(0.0f, 64.0f),
                    ImGuiChildFlags_Borders);
  ImGui::TextUnformatted(name);
  ImGui::Spacing();
  ImGui::TextDisabled("Component");
  ImGui::SameLine();
  ImGui::TextDisabled("%s", uuid.to_string().c_str());
  ImGui::EndChild();
  ImGui::Dummy({ 0.0f, SectionSpacing });

  draw_component<TransformComponent>("Transform", entity, [&](auto& component) {
    auto changed = draw_vec3_row("Location", component.position, 0.1f);
    changed |= draw_vec3_row("Rotation", component.rotation, 1.0f);
    changed |= draw_vec3_row("Scale", component.scale, 0.1f);
    return changed;
  });

  draw_component<SkyboxComponent>("Skybox", entity, [&](auto& component) {
    auto& env = component.env;
    auto changed =
        draw_combo_row("Mode", (int*)&env.mode, "Solid Color\0Texture\0\0");
    switch (env.mode) {
      case LightingMode::SolidColor:
        changed |= draw_property_row(env.color);
        break;
      case LightingMode::Texture:
        changed |= draw_property_row(env.hdr);
        break;
    }
    return changed;
  });

  draw_component<StaticMeshComponent>(
      "Static Mesh", entity, [&](auto& component) {
        auto changed = false;
        begin_row("Mesh");
        ImGui::Button(component.mesh ? "Assigned Mesh" : "Drop Mesh Here",
                      ImVec2(-1.0f, 0.0f));
        if (ImGui::BeginDragDropTarget()) {
          if (const auto* payload =
                  ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
            auto path = static_cast<const char*>(payload->Data);
            auto mesh = Mesh();
            mesh.read(fs::path(path));
            component.mesh = shared<Mesh>(mesh);
            changed = true;
          }
          ImGui::EndDragDropTarget();
        }
        end_row();
        return changed;
      });

  draw_component<LightComponent>("Light", entity, [&](auto& component) {
    auto changed =
        draw_combo_row("Type", (int*)&component.kind, "Point\0Sun\0\0");
    changed |= draw_float_row("A", &component.a);
    changed |= draw_float_row("B", &component.b);
    return changed;
  });

  if (ImGui::Button("Add Component", ImVec2(-1.0f, 32.0f))) {
    ImGui::OpenPopup("AddComponent");
  }
  if (ImGui::BeginPopup("AddComponent")) {
    draw_add_component<StaticMeshComponent>(entity, "Static Mesh");
    draw_add_component<SkyboxComponent>(entity, "Skybox");
    draw_add_component<LightComponent>(entity, "Light");
    ImGui::EndPopup();
  }

  ImGui::PopStyleColor(5);
  ImGui::PopStyleVar(5);
}

} // namespace zod
