#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "application/application.hh"
#include "config.hh"
#include "imgui_layer.hh"
#include "theme.hh"

namespace zod {

static String g_imgui_ini_path;

ImGuiLayer::ImGuiLayer(void* win) {
  auto* window = (GLFWwindow*)win;
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  auto& io = ImGui::GetIO();
  g_imgui_ini_path = (find_config_dir() / "imgui.ini").string();
  io.IniFilename = g_imgui_ini_path.c_str();
  ImGui::LoadIniSettingsFromDisk(io.IniFilename);
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 450");
  io.FontDefault = io.Fonts->AddFontFromFileTTF(
      "../third-party/imgui/misc/fonts/DroidSans.ttf", 14.0f);
  Theme::load();
  init_theme();
}

void ImGuiLayer::begin_frame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
  if (m_opt_fullscreen) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const auto title_bar_height =
        Application::get().info().window_info.custom_title_bar
            ? Window::TitleBarHeight
            : 0;
    ImGui::SetNextWindowPos(
        ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + title_bar_height));
    ImGui::SetNextWindowSize(
        ImVec2(viewport->WorkSize.x,
               std::max(0.0f, viewport->WorkSize.y - title_bar_height)));
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |=
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  }

  if (m_dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
    window_flags |= ImGuiWindowFlags_NoBackground;
  }

  if (not m_opt_padding) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  }

  m_open = true;

  auto& io = ImGui::GetIO();
  ImGui::Begin("DockSpace", &m_open, window_flags);

  if (not m_opt_padding) {
    ImGui::PopStyleVar();
  }

  if (m_opt_fullscreen) {
    ImGui::PopStyleVar(2);
  }

  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    auto dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), m_dockspace_flags);
  }

  ImGui::End();
}

void ImGuiLayer::end_frame() {
  auto& io = ImGui::GetIO();
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}

ImGuiLayer::~ImGuiLayer() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

}; // namespace zod
