#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "imgui_layer.hh"

namespace zod {

ImGuiLayer::ImGuiLayer(GLFWwindow* window) {
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

  auto& colors = style.Colors;
  auto pink = ImVec4 { 1.0f, 0.0f, 1.0f, 1.0f };

  auto bg = ImVec4 { 0.07f, 0.08f, 0.08f, 1.0f };
  auto fg = ImVec4 { 0.2f, 0.22f, 0.23f, 1.0f };
  auto hover = ImVec4 { 0.3f, 0.32f, 0.32f, 1.0f };
  auto active = ImVec4 { 0.15f, 0.16f, 0.17f, 1.0f };

  colors[ImGuiCol_WindowBg] = bg;
  colors[ImGuiCol_Header] = fg;
  colors[ImGuiCol_HeaderHovered] = hover;
  colors[ImGuiCol_HeaderActive] = active;

  colors[ImGuiCol_Button] = fg;
  colors[ImGuiCol_ButtonHovered] = hover;
  colors[ImGuiCol_ButtonActive] = active;

  colors[ImGuiCol_FrameBg] = active;
  colors[ImGuiCol_FrameBgHovered] = fg;
  colors[ImGuiCol_FrameBgActive] = active;

  colors[ImGuiCol_Tab] = pink;
  colors[ImGuiCol_TabHovered] = active;
  colors[ImGuiCol_TabActive] = active;
  colors[ImGuiCol_TabUnfocused] = pink;
  colors[ImGuiCol_TabUnfocusedActive] = active;

  colors[ImGuiCol_TitleBg] = bg;
  colors[ImGuiCol_TitleBgActive] = bg;
  colors[ImGuiCol_TitleBgCollapsed] = hover;

  colors[ImGuiCol_SliderGrab] = hover;
  colors[ImGuiCol_SliderGrabActive] = hover;
}

void ImGuiLayer::begin_frame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
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
