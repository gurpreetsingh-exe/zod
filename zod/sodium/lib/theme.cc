#include <fstream>
#include <imgui.h>
#include <nlohmann/json.hpp>

#include "config.hh"
#include "theme.hh"

namespace zod {

using json = nlohmann::json;

Theme g_theme;

#define ARRAY_TO_VEC4(jsonprop, prop)                                          \
  {                                                                            \
    float v[4] = {};                                                           \
    j.at(jsonprop).get_to(v);                                                  \
    prop = vec4(v[0], v[1], v[2], v[3]);                                       \
  }

static auto from_json(const json& j, Theme& theme) -> void {
  ARRAY_TO_VEC4("background", theme.background);
  ARRAY_TO_VEC4("highlight", theme.highlight);
  ARRAY_TO_VEC4("primary", theme.primary);
}

auto Theme::load() -> void {
  auto config = find_config_dir() / "editor.json";
  auto ss = std::ifstream(config);
  auto j = json::parse(ss);
  g_theme = j["theme"].template get<Theme>();
}

auto Theme::get() -> Theme& { return g_theme; }

auto active = ImVec4 { 0.15f, 0.16f, 0.17f, 1.0f };
auto tab_inactive = ImVec4 { 0.1f, 0.12f, 0.13f, 1.0f };
auto hover = ImVec4 { 0.3f, 0.32f, 0.32f, 1.0f };

auto init_theme() -> void {
  auto& style = ImGui::GetStyle();
  style.WindowMenuButtonPosition = ImGuiDir_None;
  style.TabBarOverlineSize = 0;

  style.WindowBorderSize = 0.0f;
  style.FrameRounding = 3.f;
  style.ChildRounding = 3.f;
  style.CellPadding = ImVec2(6.0f, 2.0f);

  auto& colors = style.Colors;
  auto& theme = Theme::get();
  colors[ImGuiCol_WindowBg] = *reinterpret_cast<ImVec4*>(&theme.background);
  colors[ImGuiCol_Header] = *reinterpret_cast<ImVec4*>(&theme.highlight);
  colors[ImGuiCol_HeaderHovered] = *reinterpret_cast<ImVec4*>(&theme.highlight);
  colors[ImGuiCol_HeaderActive] = *reinterpret_cast<ImVec4*>(&theme.highlight);

  colors[ImGuiCol_Button] = *reinterpret_cast<ImVec4*>(&theme.highlight);
  colors[ImGuiCol_ButtonHovered] = *reinterpret_cast<ImVec4*>(&theme.highlight);
  colors[ImGuiCol_ButtonActive] = active;

  colors[ImGuiCol_FrameBg] = active;
  colors[ImGuiCol_FrameBgHovered] = active;
  colors[ImGuiCol_FrameBgActive] = active;

  colors[ImGuiCol_Tab] = tab_inactive;
  colors[ImGuiCol_TabHovered] = active;
  colors[ImGuiCol_TabActive] = active;
  colors[ImGuiCol_TabUnfocused] = tab_inactive;
  colors[ImGuiCol_TabUnfocusedActive] = active;

  colors[ImGuiCol_TitleBg] = *reinterpret_cast<ImVec4*>(&theme.background);
  colors[ImGuiCol_TitleBgActive] =
      *reinterpret_cast<ImVec4*>(&theme.background);
  colors[ImGuiCol_TitleBgCollapsed] = hover;

  colors[ImGuiCol_SliderGrab] = hover;
  colors[ImGuiCol_SliderGrabActive] = hover;

  colors[ImGuiCol_MenuBarBg] = *reinterpret_cast<ImVec4*>(&theme.highlight);
  colors[ImGuiCol_PopupBg] = *reinterpret_cast<ImVec4*>(&theme.background);
}

} // namespace zod
