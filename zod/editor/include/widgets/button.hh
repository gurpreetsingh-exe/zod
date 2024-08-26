#pragma once

#include <imgui.h>

namespace zod {

constexpr auto padding = 4.0f;
constexpr auto inner = 4.0f;

inline auto Button(const char* name, bool& enabled) -> void {
  ImGui::SetNextItemAllowOverlap();
  auto button_size = ImGui::CalcTextSize(name);
  auto cursor = ImVec2(ImGui::GetWindowContentRegionMax().x - button_size.x -
                           padding - inner * 2,
                       ImGui::GetWindowContentRegionMin().y + padding);
  ImGui::SetCursorPos(cursor);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(inner, inner));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.f);
  if (enabled) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0.45f, 0.82f, 1));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0.35f, 0.82f, 1));
  }
  ImGui::Button(name);
  if (enabled) {
    ImGui::PopStyleColor(2);
  }
  if (ImGui::IsItemClicked(0)) {
    enabled = not enabled;
  }
  ImGui::PopStyleVar(2);
}

} // namespace zod
