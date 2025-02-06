#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "imgui_layer.hh"

#include "application.hh"

namespace zod {

auto pink = ImVec4 { 1.0f, 0.0f, 1.0f, 1.0f };
auto bg = ImVec4 { 0.07f, 0.08f, 0.08f, 1.0f };
auto fg = ImVec4 { 0.2f, 0.22f, 0.23f, 1.0f };
auto active = ImVec4 { 0.15f, 0.16f, 0.17f, 1.0f };
auto tab_inactive = ImVec4 { 0.1f, 0.12f, 0.13f, 1.0f };
// #0070e0
auto blue = ImVec4 { 0.0f, 0.44f, 0.878f, 1.0f };
auto hover = ImVec4 { 0.3f, 0.32f, 0.32f, 1.0f };

ImGuiLayer::ImGuiLayer(void* win) {
  auto* window = (GLFWwindow*)win;
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  auto& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 450");
  io.FontDefault = io.Fonts->AddFontFromFileTTF(
      "../third-party/imgui/misc/fonts/DroidSans.ttf", 14.0f);

  auto& style = ImGui::GetStyle();
  style.WindowMenuButtonPosition = ImGuiDir_None;
  style.TabBarOverlineSize = 0;

  style.WindowBorderSize = 0.0f;
  style.FrameRounding = 3.f;
  style.ChildRounding = 3.f;
  style.CellPadding = ImVec2(6.0f, 2.0f);

  auto& colors = style.Colors;

  // colors[ImGuiCol_Border] = active;
  colors[ImGuiCol_WindowBg] = bg;
  colors[ImGuiCol_Header] = fg;
  colors[ImGuiCol_HeaderHovered] = fg;
  colors[ImGuiCol_HeaderActive] = fg;

  colors[ImGuiCol_Button] = fg;
  colors[ImGuiCol_ButtonHovered] = fg;
  colors[ImGuiCol_ButtonActive] = active;

  colors[ImGuiCol_FrameBg] = active;
  colors[ImGuiCol_FrameBgHovered] = active;
  colors[ImGuiCol_FrameBgActive] = active;

  colors[ImGuiCol_Tab] = tab_inactive;
  colors[ImGuiCol_TabHovered] = active;
  colors[ImGuiCol_TabActive] = active;
  colors[ImGuiCol_TabUnfocused] = tab_inactive;
  colors[ImGuiCol_TabUnfocusedActive] = active;

  colors[ImGuiCol_TitleBg] = bg;
  colors[ImGuiCol_TitleBgActive] = bg;
  colors[ImGuiCol_TitleBgCollapsed] = hover;

  colors[ImGuiCol_SliderGrab] = hover;
  colors[ImGuiCol_SliderGrabActive] = hover;

  colors[ImGuiCol_MenuBarBg] = fg;
  colors[ImGuiCol_PopupBg] = fg;
}

void ImGuiLayer::begin_frame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  if (m_opt_fullscreen) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    m_window_flags |= ImGuiWindowFlags_NoTitleBar |
                      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                      ImGuiWindowFlags_NoMove;
    m_window_flags |=
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  }

  if (m_dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
    m_window_flags |= ImGuiWindowFlags_NoBackground;
  }

  if (not m_opt_padding) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  }

  m_open = true;

  auto& io = ImGui::GetIO();
  ImGui::Begin("DockSpace", &m_open, m_window_flags);

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

  ImGui::PushStyleColor(ImGuiCol_Border, fg);
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("New")) {
        TODO();
      }
      if (ImGui::BeginMenu("Open")) {
        ImGui::MenuItem("Recent");
        // TODO();
        ImGui::EndMenu();
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Save")) {
        TODO();
      }
      if (ImGui::MenuItem("Exit")) {
        SApplication::get().active_window().close();
      }
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
  ImGui::PopStyleColor();
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
