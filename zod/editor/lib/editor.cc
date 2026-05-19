#include "editor.hh"

#include "application/platform.hh"
#include "content_browser.hh"
#include "engine/components.hh"
#include "engine/project.hh"
#include "gpu/timer.hh"
#include "loaders.hh"
#include "outliner.hh"
#include "properties.hh"
#include "sodium/custom_gui.hh"
#include "sodium/theme.hh"
#include "viewport.hh"

#include "application/input.hh"

#include "core/option.hh"

#include "sodium/gui.hh"

namespace zod {

Editor* g_editor;
namespace sodium {
extern mat4 projection;
auto resize(vec2) -> void;
} // namespace sodium

static constexpr usize MAX_PATH = 256;
static auto preferences = false;

auto Editor::get() -> Editor& { return *g_editor; }

Editor::Editor()
    : ILayer(), m_renderer(new ForwardRenderer()),
      m_node_tree(shared<NodeTree>()) {
  g_editor = this;
  // m_widget = sodium::create<sodium::container>().background();

  auto light = vec4 { 0.127, 0.131, 0.14, 1 };
  auto dark = vec4 { 0.0825, 0.084, 0.09, 1 };

  using namespace sodium;

  auto button = [](f32 width = 50, IconId icon = IconId::None,
                   const std::string& name = "") {
    return slot().auto_width()[create<Button>(icon)
                                   .min_size({ width, 0 })
                                   .content_padding({ 8, 8, 0, 0 })
                                   .name(name)];
  };

  auto close = slot().auto_width()[create<Button>(IconId::Close)
                                       .min_size({ 50, 0 })
                                       .button_style({ { 0.9, 0.2, 0.2, 1.0 } })
                                       .background({ 0.1, 0.1, 0.1, 1.0 })];
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

  auto menus = create<HorizontalBox>() +
               slot().fixed_height(
                   25)[create<HorizontalBox>() + menu("File") + menu("Edit")];
  auto spread = slot()[create<Box>()];

  auto logo = GPUBackend::get().create_texture(
      { .name = "zod-logo", .path = "./logo.png" });

  auto window_buttons = slot().auto_width().fixed_height(
      30)[create<HorizontalBox>() + minimize + maximize + close];

  constexpr auto padding = 4.0f;
  auto titlebar =
      create<HorizontalBox>().background(dark) +
      slot().fixed_width(40)[create<Image>(logo).background(light)] +
      slot()[menus] + spread + window_buttons;

  auto status_bar = create<Box>().background(light).name("StatusBar");

  auto tabs =
      create<HorizontalBox>().gap(padding) +
      slot()[create<VerticalBox>().gap(padding) +
             slot()[create<HorizontalBox>().gap(padding) +
                    slot().fixed_width(250)[create<Box>().background(light)] +
                    slot()[create<Box>().background(light)]] +
             slot().fixed_height(250)[create<Box>().background(light)]] +
      slot().fixed_width(320)[create<Box>().background(light)];
  // slot().fixed_width(200)[create<Box>().background(light).name("A")] +
  // slot()[create<Box>().background(light).name("B")] +
  // slot().fixed_width(200)[create<Box>().background(light).name("C")];

  auto window =
      create<VerticalBox>() + slot().fixed_height(40)[titlebar] +
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
    // editor_gui().layout({ { 0.0f, 0.0f }, event.size });
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
