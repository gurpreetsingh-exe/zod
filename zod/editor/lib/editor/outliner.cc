#include "outliner.hh"
#include "engine/components.hh"
#include "engine/project.hh"
#include "sodium/theme.hh"
#include "sodium/widgets/box.hh"
#include "sodium/widgets/builder.hh"
#include "sodium/widgets/text.hh"

namespace zod {

static constexpr auto TextPadding = 12.0f;

Outliner::Outliner() : m_list(shared<sodium::MultiColumnListView<Entity>>()) {
  using namespace sodium;

  m_list->set_items_source([] {
    if (not g_project) {
      return Vector<Entity>();
    }
    return g_project->active_scene()->entities();
  });

  m_list->set_header_background(Theme::get().foreground);
  m_list->set_splitter_thickness(2.0f);
  m_list->set_splitter_widget(
      *create<Box>().background(Theme::get().foreground));
  m_list->set_row_background([](usize row) {
    auto& theme = Theme::get();
    return row % 2 == 0 ? theme.background - 0.01f : theme.background + 0.01f;
  });
  m_list->add_column(
      "Name",
      [](Entity entity, usize) {
        const auto& identifier = entity.get_component<IdentifierComponent>();
        return text_cell(identifier.identifier);
      },
      0.6f);
  m_list->add_column(
      "Type", [](Entity entity, usize) { return text_cell(type_name(entity)); },
      0.4f);

  set_child(m_list);
}

auto Outliner::request_refresh() -> void { m_list->request_refresh(); }

auto Outliner::type_name(Entity entity) -> const char* {
  if (entity.has_component<StaticMeshComponent>()) {
    return "Mesh";
  }
  if (entity.has_component<LightComponent>()) {
    return "Light";
  }
  if (entity.has_component<CameraComponent>()) {
    return "Camera";
  }
  if (entity.has_component<SkyboxComponent>()) {
    return "Skybox";
  }
  return "Object";
}

auto Outliner::text_cell(String text) -> SharedPtr<sodium::Widget> {
  using namespace sodium;
  return *create<Box>().content_valign(
      Align::Center)[create<Text>(std::move(text))
                         .padding({ TextPadding, TextPadding, 0, 0 })];
}

} // namespace zod
