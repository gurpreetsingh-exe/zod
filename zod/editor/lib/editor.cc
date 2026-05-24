#include "editor.hh"
#include "core/platform/macros.hh"

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

namespace zod {

Editor* g_editor;
namespace sodium {
extern mat4 projection;
auto resize(vec2) -> void;
} // namespace sodium

static constexpr usize MAX_PATH = 256;
static auto preferences = false;

auto Editor::get() -> Editor& { return *g_editor; }

auto light = vec4 { 0.127, 0.131, 0.14, 1 };
auto dark = vec4 { 0.0825, 0.084, 0.09, 1 };

Editor::Editor()
    : ILayer(), m_renderer(new ForwardRenderer()),
      m_node_tree(shared<NodeTree>()) {
  g_editor = this;
  // m_widget = sodium::create<sodium::container>().background();

  using namespace sodium;

  auto button = [](f32 width = 50, IconId icon = IconId::None,
                   const std::string& name = "") {
    return slot().auto_width()[create<Button>(icon)
                                   .min_size({ width, 0 })
                                   .content_padding({ 8, 8, 0, 0 })
                                   .name(name)];
  };

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

  auto menu = [](const String& name) {
    return slot().auto_width()
        [create<Menu>().min_size({ 40, 0 }).padding({ 8, 8, 0, 0 }).name(name)];
  };

  auto menus =
      create<HorizontalBox>() +
      slot()
          .fixed_height(25)
          .auto_width()[create<HorizontalBox>() + menu("File") + menu("Edit")];
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

  constexpr auto padding = 4.0f;
  auto titlebar =
      create<HorizontalBox>().background(dark) +
      slot().fixed_width(
          Window::TitleBarHeight)[create<Image>(logo).background(light)] +
      slot().auto_width()[menus] + spread + window_buttons;

  auto status_bar = create<Box>().background(light).name("StatusBar");

  // TABS OLD
  // auto tabs =
  //     create<HorizontalBox>().gap(padding) +
  //     slot()[create<VerticalBox>().gap(padding) +
  //            slot()[create<HorizontalBox>().gap(padding) +
  //                   slot().fixed_width(250)[create<Box>().background(light)]
  //                   + slot()[create<Box>().background(light)]] +
  //            slot().fixed_height(250)[create<Box>().background(light)]] +
  //     slot().fixed_width(320)[create<Box>().background(light)];

  struct Split {
    struct ResizeState {
      bool resizing = false;
      vec2 start_mouse = {};
      f32 start_split = 0.5f;
      f32 split = 0.5f;
    };

    Split(SharedPtr<Widget> first_, SharedPtr<Widget> second_, f32 s = 0.5f)
        : split(s),
          splitter(create<Box>()
                       .hit_test_margin(Padding { 8, 8, 0, 0 })
                       .hit_test_priority(10)
                       .cursor(cursor_shape_t::ResizeHorizontal)
                       .build()),
          inner(create<HorizontalBox>() + slot().fill_width(split)[first_] +
                slot().fixed_width(padding)[splitter] +
                slot().fill_width(1.0f - split)[second_]),
          first(inner->get_slot(0)), second(inner->get_slot(2)) {
      auto state = shared<ResizeState>();
      state->split = split;
      auto inner_box = inner.get();
      auto first_slot = &first;
      auto second_slot = &second;

      auto apply_split = [state, inner_box, first_slot,
                          second_slot](f32 value) {
        state->split = glm::clamp(value, 0.05f, 0.95f);
        first_slot->style.horizontal_stretch_weight = state->split;
        second_slot->style.horizontal_stretch_weight = 1.0f - state->split;
        inner_box->invalidate_layout();
      };

      splitter->set_on_mouse_down([state](const Event& event) {
        if (event.button != MouseButton::Left) {
          return EventResponse::unhandled();
        }
        state->resizing = true;
        state->start_mouse = event.mouse;
        state->start_split = state->split;
        return EventResponse::handled().capture_mouse(MouseButton::Left);
      });

      splitter->set_on_mouse_move(
          [state, inner_box, apply_split](const Event& event) {
            if (not state->resizing) {
              return EventResponse::unhandled();
            }

            auto width = std::max(1.0f, inner_box->frame().size.x);
            auto delta = event.mouse.x - state->start_mouse.x;
            apply_split(state->start_split + delta / width);
            return EventResponse::handled();
          });

      splitter->set_on_mouse_up([state](const Event& event) {
        if (event.button == MouseButton::Left and state->resizing) {
          state->resizing = false;
          return EventResponse::handled();
        }
        return EventResponse::unhandled();
      });
    }

    f32 split = 0.5f;
    SharedPtr<Widget> splitter = nullptr;
    SharedPtr<HorizontalBox> inner = nullptr;
    Container::Slot& first;
    Container::Slot& second;
  };

  auto tabs = Split(create<Box>().background(light).build(),
                    create<Box>().background(light).build())
                  .inner;

  auto window =
      create<VerticalBox>() +
      slot().fixed_height(Window::TitleBarHeight)[titlebar] +
      slot()[create<VerticalBox>().padding(padding).gap(padding) +
             slot()[tabs]] +
      slot().padding({ padding, padding, 0, 0 }).fixed_height(30)[status_bar];

  m_widget =
      create<CompoundWidget>()
          .background(light)[create<CompoundWidget>().padding(2).background(
              dark)[window]]
          .build();
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

  // m_layout->add_area(shared<Viewport>());
  // m_layout->add_area(shared<Properties>());
  // m_layout->add_area(shared<Outliner>());
  // m_layout->add_area(shared<ContentBrowser>());

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
    // editor_gui().layout({ { 0.0f, 0.0f }, event.size });
  }

  auto reply = m_widget->event(event);
  if (reply) {
    event.hanging = false;
  }

  // editor_gui().on_event(event);

  // if (auto* area = m_layout->active()) {
  //   area->on_event(event);
  // }
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
    if (m_widget->needs_layout()) {
      m_widget->compute_desired_size(size);
      m_widget->arrange({ { 0, 0 }, size });
      m_widget->clear_layout_invalidated();
    }

    GPUState::get().set_blend(Blend::Alpha);
    m_widget->paint(cx);
    // editor_gui().paint(cx);
    sodium::submit(cx);
    sodium::flush_icons();
    sodium::draw_images();
    sodium::Font::get().submit();
    sodium::present();
    GPUState::get().set_blend(Blend::None);

    // m_imgui_layer->begin_frame();
    //
    // auto& theme = Theme::get();
    // ImGui::PushStyleColor(ImGuiCol_Border,
    //                       *reinterpret_cast<ImVec4*>(&theme.highlight));
    // ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
    //                       *reinterpret_cast<ImVec4*>(&theme.primary));
    //
    // static auto open_save_modal = false;
    // if (ImGui::BeginMainMenuBar()) {
    //   if (ImGui::BeginMenu("File")) {
    //     if (ImGui::MenuItem("New")) {
    //       TODO();
    //     }
    //     if (ImGui::MenuItem("Open")) {
    //       auto config_path = open_dialog({ .filter = "*.zproj" });
    //       if (g_project) {
    //         delete g_project;
    //       }
    //       Project::load(config_path);
    //       update_viewport_camera();
    //     }
    //     ImGui::Separator();
    //     if (ImGui::MenuItem("Save")) {
    //       if (g_project) {
    //         g_project->save();
    //       } else {
    //         open_save_modal = true;
    //       }
    //     }
    //
    //     ImGui::Separator();
    //     if (ImGui::BeginMenu("Import")) {
    //       if (ImGui::MenuItem("GLTF")) {
    //         auto gltf_path = open_dialog({ .filter = "*.gltf *.glb" });
    //         loadGLTF(g_project->assets_directory(), gltf_path);
    //       }
    //       ImGui::EndMenu();
    //     }
    //     if (ImGui::MenuItem("Quit")) {
    //       Event event = { .kind = Event::WindowClose };
    //       Application::get().on_event(event);
    //     }
    //     ImGui::EndMenu();
    //   }
    //
    //   if (ImGui::BeginMenu("Edit")) {
    //     if (ImGui::MenuItem("Preferences")) {
    //       preferences = true;
    //     }
    //     ImGui::EndMenu();
    //   }
    //
    //   ImGui::EndMainMenuBar();
    //   // } else {
    //   //   ImGui::End();
    // }
    // ImGui::PopStyleColor(2);
    //
    // if (open_save_modal) {
    //   ImGui::OpenPopup("Save Project");
    // }
    //
    // auto center = ImGui::GetMainViewport()->GetCenter();
    // ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    //
    // if (ImGui::BeginPopupModal("Save Project", NULL,
    //                            ImGuiWindowFlags_AlwaysAutoResize)) {
    //   const char* name_l = "Project Name";
    //   const char* loc_l = "Project Location";
    //   const auto padding = ImGui::CalcTextSize(loc_l, NULL, true).x + 20;
    //
    //   // Project Name
    //   ImGui::BeginGroup();
    //   ImGui::AlignTextToFramePadding();
    //   ImGui::Text("%s", name_l);
    //   ImGui::SameLine(padding);
    //   static char nbuf[MAX_PATH] = {};
    //   ImGui::InputText("##Name", nbuf, MAX_PATH);
    //   ImGui::EndGroup();
    //
    //   // Project Location
    //   ImGui::BeginGroup();
    //   ImGui::AlignTextToFramePadding();
    //   ImGui::Text("%s", loc_l);
    //   ImGui::SameLine(padding);
    //   static char buf[MAX_PATH] = {};
    //   ImGui::InputText("##Location", buf, MAX_PATH);
    //   ImGui::SameLine();
    //   if (ImGui::Button("...")) {
    //     auto path = open_dialog({ .selection_mode = SelectionMode::Directory
    //     }); ZASSERT(path.size() < MAX_PATH); memcpy(buf, path.c_str(),
    //     path.size());
    //   }
    //   ImGui::EndGroup();
    //
    //   auto align = [](float width, float alignment = 0.5f) {
    //     auto avail = ImGui::GetContentRegionAvail().x;
    //     auto off = (avail - width) * alignment;
    //     if (off > 0.0f) {
    //       ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
    //     }
    //   };
    //
    //   auto& style = ImGui::GetStyle();
    //   const auto s = ImVec2(120, 0);
    //   f32 width = 0.0f;
    //   width += s.x * 2.0f;
    //   width += style.ItemSpacing.x;
    //   align(width);
    //
    //   if (ImGui::Button("Save", s)) {
    //     g_project = new Project(nbuf, buf);
    //     g_project->init();
    //     g_project->save();
    //
    //     open_save_modal = false;
    //     ImGui::CloseCurrentPopup();
    //   }
    //
    //   ImGui::SameLine();
    //   if (ImGui::Button("Cancel", s)) {
    //     memset(buf, 0, MAX_PATH);
    //     memset(nbuf, 0, MAX_PATH);
    //     open_save_modal = false;
    //     ImGui::CloseCurrentPopup();
    //   }
    //
    //   ImGui::EndPopup();
    // }
    //
    // if (preferences) {
    //   ImGui::Begin("Preferences", &preferences,
    //                ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse);
    //   ImGui::SeparatorText("Theme");
    //   bool update = false;
    //   update |= ImGui::ColorEdit3("Background", &theme.background.x,
    //                               ImGuiColorEditFlags_Float);
    //   update |= ImGui::ColorEdit3("Highlights", &theme.highlight.x,
    //                               ImGuiColorEditFlags_Float);
    //   if (update) {
    //     init_theme();
    //   }
    //   ImGui::End();
    // }
    //
    // m_imgui_layer->update([&] {
    //   ImGui::Begin("DebugEditor");
    //   ImGuiIO& io = ImGui::GetIO();
    //   ImGui::Text("Delta Time: %.3f ms", 1000.0f / io.Framerate);
    //   static bool vsync = true;
    //   if (ImGui::Checkbox("V-Sync", &vsync)) {
    //     Application::get().active_window().set_vsync(vsync);
    //   }
    //   ImGui::Separator();
    //
    //   // auto viewport =
    //   //     std::static_pointer_cast<Viewport>(m_layout->area("Viewport"));
    //   // ImGui::Checkbox("G-Buffer", std::addressof(viewport->gbuffer()));
    //   // ImGui::SliderInt("G-Buffer Slot",
    //   //                  std::addressof(viewport->gbuffer_slot()), 0, 1);
    //
    //   ImGui::Separator();
    //   for (const auto& [name, time] : GPUTimer::get().timings()) {
    //     ImGui::Text("%s: %.3f ms", name.c_str(), time);
    //   }
    //
    //   ImGui::End();
    //
    //   // static auto g = Geometry();
    //   // m_layout->draw(g);
    // });
    // m_imgui_layer->end_frame();
  });
}

} // namespace zod
