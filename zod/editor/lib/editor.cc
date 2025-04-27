#include "editor.hh"

#include "application/platform.hh"
#include "content_browser.hh"
#include "engine/components.hh"
#include "engine/runtime.hh"
#include "gpu/timer.hh"
#include "loaders.hh"
#include "node_editor.hh"
#include "node_properties.hh"
#include "outliner.hh"
#include "properties.hh"
#include "theme.hh"
#include "viewport.hh"
#include "widgets/layout.hh"

namespace zod {

Editor* g_editor;

static constexpr usize MAX_PATH = 256;
static auto preferences = false;

auto Editor::get() -> Editor& { return *g_editor; }

Editor::Editor()
    : ILayer(), m_renderer(new ForwardRenderer()),
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

  m_project = Project::load(path);

  m_layout->add_area(shared<Viewport>());
  m_layout->add_area(shared<NodeEditor>());
  m_layout->add_area(shared<Properties>());
  m_layout->add_area(shared<NodeProperties>());
  m_layout->add_area(shared<Outliner>());
  m_layout->add_area(shared<ContentBrowser>());

  update_viewport_camera();
}

auto Editor::on_event(Event& event) -> void {
  if (auto* area = m_layout->active()) {
    area->on_event(event);
  }
}

auto Editor::update_viewport_camera() -> void {
  auto viewport = m_layout->area("Viewport");
  auto& scene = Runtime::get().scene();
  auto entity = scene.active_camera();
  auto camera = entity.get_component<CameraComponent>().camera;
  auto size = viewport->get_size();
  camera->resize(size.x, size.y);
  camera->update_matrix();
  viewport->set_camera(camera);
}

auto Editor::update() -> void {
  Runtime::get().scene().update();

  GPU_TIME("main-loop", {
    m_imgui_layer->begin_frame();

    auto& theme = Theme::get();
    ImGui::PushStyleColor(ImGuiCol_Border,
                          *reinterpret_cast<ImVec4*>(&theme.highlight));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                          *reinterpret_cast<ImVec4*>(&theme.primary));
    static auto open_save_modal = false;
    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New")) {
          TODO();
        }
        if (ImGui::MenuItem("Open")) {
          auto config_path = open_dialog({ .filter = "*.zproj" });
          if (m_project) {
            delete m_project;
          }
          m_project = Project::load(config_path);
          update_viewport_camera();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Save")) {
          if (m_project) {
            m_project->save();
          } else {
            open_save_modal = true;
          }
        }

        ImGui::Separator();
        if (ImGui::BeginMenu("Import")) {
          if (ImGui::MenuItem("GLTF")) {
            auto gltf_path = open_dialog({ .filter = "*.gltf *.glb" });
            loadGLTF(m_project->assets_directory(), gltf_path);
          }
          ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Quit")) {
          Event event = { .kind = Event::WindowClose };
          Application::get().on_event(event);
        }
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Preferences")) {
          preferences = true;
        }
        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }
    ImGui::PopStyleColor(2);

    if (open_save_modal) {
      ImGui::OpenPopup("Save Project");
    }

    auto center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Save Project", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      const char* name_l = "Project Name";
      const char* loc_l = "Project Location";
      const auto padding = ImGui::CalcTextSize(loc_l, NULL, true).x + 20;

      // Project Name
      ImGui::BeginGroup();
      ImGui::AlignTextToFramePadding();
      ImGui::Text(name_l);
      ImGui::SameLine(padding);
      static char nbuf[MAX_PATH] = {};
      ImGui::InputText("##Name", nbuf, MAX_PATH);
      ImGui::EndGroup();

      // Project Location
      ImGui::BeginGroup();
      ImGui::AlignTextToFramePadding();
      ImGui::Text(loc_l);
      ImGui::SameLine(padding);
      static char buf[MAX_PATH] = {};
      ImGui::InputText("##Location", buf, MAX_PATH);
      ImGui::SameLine();
      if (ImGui::Button("...")) {
        auto path = open_dialog({ .selection_mode = SelectionMode::Directory });
        ZASSERT(path.size() < MAX_PATH);
        memcpy(buf, path.c_str(), path.size());
      }
      ImGui::EndGroup();

      auto align = [](float width, float alignment = 0.5f) {
        auto avail = ImGui::GetContentRegionAvail().x;
        auto off = (avail - width) * alignment;
        if (off > 0.0f) {
          ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
        }
      };

      auto& style = ImGui::GetStyle();
      const auto s = ImVec2(120, 0);
      f32 width = 0.0f;
      width += s.x * 2.0f;
      width += style.ItemSpacing.x;
      align(width);

      if (ImGui::Button("Save", s)) {
        m_project = new Project(nbuf, buf);
        m_project->init();
        m_project->save();

        open_save_modal = false;
        ImGui::CloseCurrentPopup();
      }

      ImGui::SameLine();
      if (ImGui::Button("Cancel", s)) {
        memset(buf, 0, MAX_PATH);
        memset(nbuf, 0, MAX_PATH);
        open_save_modal = false;
        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
    }

    if (preferences) {
      ImGui::Begin("Preferences", &preferences,
                   ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse);
      ImGui::SeparatorText("Theme");
      bool update = false;
      update |= ImGui::ColorEdit3("Background", &theme.background.x,
                                  ImGuiColorEditFlags_Float);
      update |= ImGui::ColorEdit3("Highlights", &theme.highlight.x,
                                  ImGuiColorEditFlags_Float);
      if (update) {
        init_theme();
      }
      ImGui::End();
    }

    m_imgui_layer->update([&] {
      ImGui::Begin("DebugEditor");
      ImGuiIO& io = ImGui::GetIO();
      ImGui::Text("Delta Time: %.3f ms", 1000.0f / io.Framerate);
      static bool vsync = true;
      if (ImGui::Checkbox("V-Sync", &vsync)) {
        Application::get().active_window().set_vsync(vsync);
      }
      ImGui::Separator();

      auto viewport =
          std::static_pointer_cast<Viewport>(m_layout->area("Viewport"));
      ImGui::Checkbox("G-Buffer", std::addressof(viewport->gbuffer()));
      ImGui::SliderInt("G-Buffer Slot",
                       std::addressof(viewport->gbuffer_slot()), 0, 1);

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
