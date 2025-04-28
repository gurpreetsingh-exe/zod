#include <imgui.h>

#include "editor.hh"
#include "engine/components.hh"
#include "properties.hh"

namespace zod {

Properties::Properties()
    : SPanel("Properties", shared<OrthographicCamera>(64.0f, 64.0f)) {}

template <typename T, typename DrawFunc>
auto draw_component(const String& name, Entity entity, DrawFunc draw) -> void {
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
    if (draw(component)) {
      entity.update_internal<T>();
    }
    ImGui::TreePop();
  }

  ImGui::Separator();
}

extern auto DragFloat3(const char*, f32*, f32) -> bool;

template <typename T>
static auto draw_add_component(Entity entity, const String& name) -> void {
  if (not entity.has_component<T>()) {
    if (ImGui::MenuItem(name.c_str())) {
      entity.add_component<T>();
      ImGui::CloseCurrentPopup();
    }
  }
}

auto Properties::update() -> void {
  auto& C = Editor::get();
  auto entity = C.active_object();
  if (not entity) {
    return;
  }

  auto& name = entity.get_component<IdentifierComponent>().identifier;
  ImGui::SeparatorText(name);
  auto& uuid = entity.get_component<UUIDComponent>().id;
  ImGui::Text("%s", uuid.to_string().c_str());
  ImGui::Separator();

  draw_component<TransformComponent>("Transform", entity, [&](auto& component) {
    auto needs_update =
        DragFloat3("Location", (float*)&component.position, 0.1f);
    needs_update |= DragFloat3("Rotation", (float*)&component.rotation, 1.0f);
    needs_update |= DragFloat3("Scale", (float*)&component.scale, 0.1f);
    return needs_update;
  });

  draw_component<SkyboxComponent>("Skybox", entity, [&](auto& component) {
    auto& env = component.env;
    ImGui::Combo("Mode", (int*)&env.mode, "Solid Color\0Texture\0\0");
    auto needs_update = false;
    switch (env.mode) {
      case LightingMode::SolidColor: {
        needs_update |= draw_property(env.color);
      } break;
      case LightingMode::Texture: {
        needs_update |= draw_property(env.hdr);
      } break;
    }
    return needs_update;
  });

  draw_component<StaticMeshComponent>(
      "Static Mesh", entity, [&](auto& component) {
        auto needs_update = false;
        if (ImGui::BeginDragDropTarget()) {
          if (const auto* payload =
                  ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
            const char* path = (const char*)payload->Data;
            auto mesh_path = fs::path(path);
            auto mesh = Mesh();
            mesh.read(mesh_path);
            component.mesh = shared<Mesh>(mesh);
            needs_update = true;
          }
          ImGui::EndDragDropTarget();
        }
        return needs_update;
      });

  draw_component<LightComponent>("Light", entity, [&](auto& component) {
    ImGui::Combo("Type", (int*)&component.kind, "Point\0\0");
    auto needs_update = false;
    needs_update |= ImGui::DragFloat("A", &component.a);
    needs_update |= ImGui::DragFloat("B", &component.b);
    return needs_update;
  });

  ImGui::PushItemWidth(-1);

  float spcx = ImGui::GetStyle().ItemSpacing.x;
  float maxWidth = ImGui::GetContentRegionAvail().x;
  float mrw = maxWidth - spcx;
  if (ImGui::Button("Add Component", ImVec2(mrw, 30.0f))) {
    ImGui::OpenPopup("AddComponent");
  }

  if (ImGui::BeginPopup("AddComponent")) {
    // draw_add_component<CameraComponent>(entity, "Camera");
    draw_add_component<StaticMeshComponent>(entity, "Static Mesh");
    draw_add_component<SkyboxComponent>(entity, "Skybox");

    ImGui::EndPopup();
  }

  ImGui::PopItemWidth();
}

} // namespace zod
