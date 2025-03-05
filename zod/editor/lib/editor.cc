#include "editor.hh"

#include "engine/components.hh"
#include "engine/runtime.hh"
#include "gpu/timer.hh"
#include "node_editor.hh"
#include "node_properties.hh"
#include "obj.hh"
#include "outliner.hh"
#include "properties.hh"
#include "viewport.hh"
#include "widgets/layout.hh"

namespace zod {

Editor* g_editor;

auto Editor::get() -> Editor& { return *g_editor; }

Editor::Editor()
    : ILayer(), m_renderer(new Renderer()),
      m_imgui_layer(unique<ImGuiLayer>(
          Application::get().active_window().get_native_handle())),
      m_layout(unique<Layout>()), m_node_tree(shared<NodeTree>()) {
  g_editor = this;
}

Editor::~Editor() { delete m_renderer; }

auto Editor::setup() -> void {
  auto args = Application::get().args();
  if (args.size() < 2) {
    eprintln("no input file");
  }
  auto path = fs::path(args[1]);
  if (not fs::exists(path)) {
    eprintln("\"{}\" does not exist", path.string());
  }
  if (fs::is_directory(path)) {
    eprintln("\"{}\" is a directory", path.string());
  }

  auto v = shared<Viewport>();
  m_layout->add_area(v);
  m_layout->add_area(shared<NodeEditor>());
  m_layout->add_area(shared<Properties>());
  m_layout->add_area(shared<NodeProperties>());
  m_layout->add_area(shared<Outliner>());

  auto& scene = Runtime::get().scene();
  auto entity = scene.create();
  scene.set_active_camera(entity);
  entity.add_component<CameraComponent>(v->camera());

  scene.create("Cube").add_component<StaticMeshComponent>(Mesh::cube());
}

auto Editor::on_event(Event& event) -> void {
  if (auto* area = m_layout->active()) {
    area->on_event(event);
  }
}

auto Editor::update() -> void {
  Runtime::get().scene().update();

  GPU_TIME("main-loop", {
    m_imgui_layer->begin_frame();
    m_imgui_layer->update([&] {
      ImGui::Begin("DebugEditor");
      ImGuiIO& io = ImGui::GetIO();
      ImGui::Text("Delta Time: %.3f ms", 1000.0f / io.Framerate);
      static bool vsync = true;
      if (ImGui::Checkbox("V-Sync", &vsync)) {
        Application::get().active_window().set_vsync(vsync);
      }

      ImGui::Separator();
      for (const auto& [name, time] : GPUTimer::get().timings()) {
        ImGui::Text("%s: %.3f ms", name.c_str(), time);
      }

      ImGui::End();

      static auto g = Geometry();
      m_layout->draw(g);
    });
    m_imgui_layer->end_frame();
  });
}

} // namespace zod
