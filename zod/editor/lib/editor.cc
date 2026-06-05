#include "editor.hh"

#include "application/platform.hh"
#include "content_browser.hh"
#include "engine/components.hh"
#include "engine/project.hh"
#include "gpu/backend.hh"
#include "gpu/timer.hh"
#include "loaders.hh"
#include "outliner.hh"
#include "properties.hh"
#include "sodium_v2/font.hh"
#include "sodium_v2/gui.hh"
#include "sodium_v2/icon.hh"
#include "sodium_v2/paint.hh"
#include "sodium_v2/widgets/box.hh"
#include "sodium_v2/widgets/box_container.hh"
#include "sodium_v2/widgets/builder.hh"
#include "sodium_v2/widgets/button.hh"
#include "sodium_v2/widgets/image.hh"
#include "sodium_v2/widgets/menu.hh"
#include "sodium_v2/widgets/text.hh"
#include "theme.hh"
#include "viewport.hh"
#include "widgets/layout.hh"

#include "core/platform/macros.hh"

#include "core/option.hh"

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

  using namespace sodium;

  sodium::init_gui();
  auto theme = Theme::get();

  auto close =
      slot().auto_width()[create<Button>(IconId::Close)
                              .min_size({ 50, 0 })
                              .button_style({ { 0.9, 0.2, 0.2, 1.0 } })
                              .background({ 0.1, 0.1, 0.1, 1.0 })
                              .on_clicked([](auto&) {
                                auto event =
                                    Event { .kind = Event::WindowClose };
                                Application::get().on_event(event);
                                return EventResponse::handled();
                              })];
  auto minimize = slot().auto_width()[create<Button>(IconId::Minimize)
                                          .min_size({ 50, 0 })
                                          .background({ 0.1, 0.1, 0.1, 1.0 })];
  auto maximize = slot().auto_width()[create<Button>(IconId::Maximize)
                                          .min_size({ 50, 0 })
                                          .background({ 0.1, 0.1, 0.1, 1.0 })];

  auto file_menu =
      create<Menu>()
          .min_size({ 40, 0 })
          .padding({ 8, 8, 0, 0 })
          .name("File")
          .menu_entry("New", [] { TODO(); })
          .menu_entry("Open",
                      [] {
                        auto config_path = open_dialog({ .filter = "*.zproj" });
                        if (g_project) {
                          delete g_project;
                        }
                        Project::load(config_path);
                      })
          .menu_separator()
          .menu_entry("Save",
                      [] {
                        if (g_project) {
                          g_project->save();
                        }
                      })
          .menu_separator()
          .menu_entry("Import GLTF",
                      [] {
                        auto gltf_path =
                            open_dialog({ .filter = "*.gltf *.glb" });
                        loadGLTF(g_project->assets_directory(), gltf_path);
                      })
          .menu_separator()
          .menu_entry("Quit", [] {
            auto event = Event { .kind = Event::WindowClose };
            Application::get().on_event(event);
          });

  auto edit_menu = create<Menu>()
                       .min_size({ 40, 0 })
                       .padding({ 8, 8, 0, 0 })
                       .name("Edit")
                       .menu_entry("Preferences", [] { preferences = true; });

  auto menus =
      create<HorizontalBox>() +
      slot().fixed_height(25).auto_width()[create<HorizontalBox>() +
                                           slot().auto_width()[file_menu] +
                                           slot().auto_width()[edit_menu]];

  auto spread = slot()[create<Box>()
#if defined(PLATFORM_LINUX)
                           .on_mouse_down([](const Event& event) {
                             if (event.button == MouseButton::Left) {
                               Application::get().active_window().drag_start();
                             }
                             return EventResponse::handled();
                           })
#endif
  ];

  auto logo = GPUBackend::get().create_texture(
      { .name = "zod-logo", .path = "./logo.png" });

  auto window_buttons = slot().auto_width().fixed_height(
      30)[create<HorizontalBox>() + minimize + maximize + close];

  auto titlebar = create<Box>().background(
      theme.background)[create<HorizontalBox>() +
                        slot().fixed_width(Window::TitleBarHeight)
                            [create<Image>(logo).background(theme.foreground)] +
                        slot().auto_width()[menus] + spread + window_buttons];

  m_titlebar_widget = *titlebar;
}

Editor::~Editor() { delete m_renderer; }

[[noreturn]]
auto usage(const String& arg0) {
  fmt::println("Usage: {} [options] path...", arg0);
  std::exit(1);
}

auto Editor::setup() -> void {
  auto args = Application::get().args();
  const auto& arg0 = args[0];
  if (args.size() < 2) {
    usage(arg0);
  }
  auto path = fs::path(args[1]);
  if (not fs::exists(path)) {
    eprintln("\"{}\" does not exist", path.string());
  } else if (fs::is_directory(path)) {
    auto zproj = none<fs::path>();
    for (const auto f : fs::directory_iterator(path)) {
      auto code = std::error_code();
      if (f.is_regular_file(code) and f.path().extension() == ".zproj") {
        zproj = some(f.path());
      }
    }

    if (zproj) {
      Project::load(*zproj);
    } else {
      g_project = new Project("Project", path);
      g_project->init();
    }
  } else {
    Project::load(path);
  }

  m_layout->add_area(shared<Viewport>());
  m_layout->add_area(shared<Properties>());
  m_layout->add_area(shared<Outliner>());
  m_layout->add_area(shared<ContentBrowser>());

  update_viewport_camera();
  arrange_titlebar(Application::get().active_window().get_size());
}

auto Editor::on_event(Event& event) -> void {
  if (event.kind == Event::WindowResize) {
    arrange_titlebar(event.size);
  }

  if (m_titlebar_widget) {
    auto reply = m_titlebar_widget->event(event);
    if (reply) {
      event.hanging = false;
      return;
    }
  }

  if (auto* area = m_layout->active()) {
    area->on_event(event);
  }
}

auto Editor::arrange_titlebar(vec2 size) -> void {
  if (not m_titlebar_widget) {
    return;
  }

  sodium::resize(size);
  m_titlebar_widget->compute_desired_size(size);
  m_titlebar_widget->arrange(
      { { 0.0f, 0.0f }, { size.x, Window::TitleBarHeight } });
  m_titlebar_widget->clear_layout_invalidated();
}

auto Editor::paint_titlebar() -> void {
  if (not m_titlebar_widget) {
    return;
  }

  auto size = Application::get().active_window().get_size();
  if (m_titlebar_widget->needs_layout()) {
    arrange_titlebar(size);
  }

  sodium::frame([this](sodium::PaintCx& cx) {
    m_titlebar_widget->paint(cx);
  });
}

auto Editor::update_viewport_camera() -> void {
  auto viewport = m_layout->area("Viewport");
  auto scene = g_project->active_scene();
  auto entity = scene->active_camera();
  auto camera = entity.get_component<CameraComponent>().camera;
  auto size = viewport->get_size();
  camera->resize(size.x, size.y);
  camera->update_matrix();
  viewport->set_camera(camera);
}

auto Editor::update() -> void {
  g_project->active_scene()->update();

  GPU_TIME("main-loop", {
    m_imgui_layer->begin_frame();

    auto& theme = Theme::get();
    ImGui::PushStyleColor(ImGuiCol_Border,
                          *reinterpret_cast<ImVec4*>(&theme.highlight));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                          *reinterpret_cast<ImVec4*>(&theme.primary));
    static auto open_save_modal = false;
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
      ImGui::Text("%s", name_l);
      ImGui::SameLine(padding);
      static char nbuf[MAX_PATH] = {};
      ImGui::InputText("##Name", nbuf, MAX_PATH);
      ImGui::EndGroup();

      // Project Location
      ImGui::BeginGroup();
      ImGui::AlignTextToFramePadding();
      ImGui::Text("%s", loc_l);
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
        g_project = new Project(nbuf, buf);
        g_project->init();
        g_project->save();

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
    paint_titlebar();
  });
}

} // namespace zod
