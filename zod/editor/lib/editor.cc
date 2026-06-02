#include "editor.hh"
#include "application/platform.hh"
#include "core/platform/macros.hh"

#include "debug.hh"
#include "engine/project.hh"
#include "gpu/timer.hh"
#include "sodium/theme.hh"

#include "core/option.hh"

#include "sodium/font.hh"
#include "sodium/gui.hh"
#include "sodium/icon.hh"
#include "sodium/style.hh"

#include "sodium/widgets/box_container.hh"
#include "sodium/widgets/builder.hh"
#include "sodium/widgets/button.hh"
#include "sodium/widgets/menu.hh"
#include "sodium/widgets/split.hh"
#include "sodium/widgets/text.hh"

#include "outliner.hh"
#include "panel.hh"

namespace zod {

Editor* g_editor;

static constexpr usize MAX_PATH = 256;
static auto preferences = false;

auto Editor::get() -> Editor& { return *g_editor; }

static auto gpu_frame_timing_text() -> String {
  const auto& timings = GPUTimer::get().timings();
  auto it = timings.find("main-loop");
  if (it == timings.end()) {
    return "GPU: -- ms";
  }

  static auto smoothed_time = 0.0f;
  auto raw_time = it->second;
  constexpr auto alpha = 0.05f;
  smoothed_time += (raw_time - smoothed_time) * alpha;

  return fmt::format("GPU: {:.3f} ms", smoothed_time);
}

Editor::Editor()
    : ILayer(), m_renderer(new ForwardRenderer()),
      m_node_tree(shared<NodeTree>()) {
  g_editor = this;

  using namespace sodium;

  auto button = [](f32 width = 50, IconId icon = IconId::None,
                   const std::string& name = "") {
    return slot().auto_width()[create<Button>(icon)
                                   .min_size({ width, 0 })
                                   .content_padding({ 8, 8, 0, 0 })
                                   .name(name)];
  };

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

  auto status_bar =
      create<HorizontalBox>().gap(2).name("StatusBar") +
      slot()[create<Box>().background(theme.foreground)] +
      slot()
          .auto_width()[create<Box>()
                            .background(theme.foreground)
                            .min_desired_width(
                                120)[create<Text>()
                                         .text(Attribute<String>([] {
                                           return gpu_frame_timing_text();
                                         }))
                                         .justification(TextJustify::Center)]];

  auto panel = [](Vector<SharedPtr<Widget>> widgets) {
    return *create<Panel>(widgets);
  };

  m_editor_viewport = *create<EditorViewport>(*m_renderer).name("Viewport");
  m_debug = *create<Debug>();

  auto outliner = *create<Box>()
                       .background(theme.foreground)
                       .name("Outliner")[create<Box>().padding(2).background(
                           theme.background)[create<Outliner>()]];

  auto content_browser =
      *create<Box>().background(theme.foreground).name("Content Browser");

  auto properties =
      *create<Box>().background(theme.foreground).name("Properties");

  auto debug =
      *create<Box>().background(theme.foreground).name("Debug")[m_debug];

  auto tabs = create<Split>(
      *create<Split>(
          *create<Split>(panel({ outliner }), m_editor_viewport, 0.25f),
          panel({ content_browser }), Axis::Vertical, 0.75f),
      panel({ properties, debug }), 0.75f);

  auto window = create<VerticalBox>() +
                slot().fixed_height(Window::TitleBarHeight)[titlebar] +
                slot()[create<VerticalBox>().padding(S.padding).gap(S.padding) +
                       slot()[tabs]] +
                slot()
                    .padding({ S.padding, S.padding, 0, 0 })
                    .fixed_height(25)[status_bar];

  m_widget = *create<Box>().background(
      theme.foreground)[create<Box>().padding(2).background(
      theme.background)[window]];
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

  update_viewport_camera();

  // loadGLTF(
  //     m_project->assets_directory(),
  //     "/home/gurpreetsingh/Development/zod2/build-release/cornell_box.gltf");
}

auto Editor::on_event(Event& event) -> void {
  if (event.kind == Event::WindowResize) {
    auto size = event.size;
    sodium::resize(size);
    m_widget->compute_desired_size(size);
    m_widget->arrange({ { 0, 0 }, size });
    m_widget->clear_layout_invalidated();
  }

  auto reply = m_widget->event(event);
  if (reply) {
    event.hanging = false;
  }
}

auto Editor::update_viewport_camera() -> void {
  // auto viewport = m_layout->area("Viewport");
  // auto scene = g_project->active_scene();
  // auto entity = scene->active_camera();
  // auto camera = entity.get_component<CameraComponent>().camera;
  // auto size = viewport->get_size();
  // camera->resize(size.x, size.y);
  // camera->update_matrix();
  // viewport->set_camera(camera);
}

auto Editor::update() -> void {
  g_project->active_scene()->update();

  auto cx = sodium::PaintCx();
  GPU_TIME("main-loop", {
    auto& app = Application::get();
    auto size = app.active_window().get_size();
    m_debug->request_refresh();
    if (m_widget->needs_layout()) {
      m_widget->compute_desired_size(size);
      m_widget->arrange({ { 0, 0 }, size });
      m_widget->clear_layout_invalidated();
    }

    m_editor_viewport->render();

    GPUState::get().set_blend(Blend::Alpha);
    m_widget->paint(cx);
    sodium::submit(cx);
    sodium::flush_icons();
    sodium::draw_images();
    sodium::Font::get().submit(cx);
    sodium::flush_foreground(cx);
    sodium::present();
    GPUState::get().set_blend(Blend::None);
  });
}

} // namespace zod
