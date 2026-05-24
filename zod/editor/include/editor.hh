#pragma once

#include "application/application.hh"
#include "application/event.hh"
#include "asset_manager.hh"
#include "editor_viewport.hh"
#include "engine/entity.hh"
#include "engine/environment.hh"
#include "engine/node_types.hh"
#include "engine/renderer.hh"

#include "sodium/widgets/widget.hh"

namespace zod {

class Editor : public ILayer {
public:
  Editor();
  ~Editor();
  static auto get() -> Editor&;
  auto get_node_tree() -> SharedPtr<NodeTree> { return m_node_tree; }
  auto get_asset_manager() -> AssetManager& { return m_asset_manager; }
  auto get_env() -> Environment& { return m_env; }
  auto active_object() const -> Entity { return m_active_object; }
  auto set_active_object(Entity e) -> void { m_active_object = e; }
  auto get_renderer() -> Renderer& { return *m_renderer; }

private:
  auto setup() -> void override;
  auto on_event(Event&) -> void override;
  auto update() -> void override;
  auto update_viewport_camera() -> void;

private:
  Renderer* m_renderer;
  SharedPtr<EditorViewport> m_editor_viewport;
  SharedPtr<sodium::Widget> m_widget;
  // UniquePtr<ImGuiLayer> m_imgui_layer;
  SharedPtr<NodeTree> m_node_tree;
  AssetManager m_asset_manager;
  Entity m_active_object;
  Environment m_env;
};

} // namespace zod
