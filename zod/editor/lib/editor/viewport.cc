#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "editor.hh"
#include "engine/camera.hh"
#include "engine/runtime.hh"
#include "gpu/timer.hh"
#include "viewport.hh"
#include "widgets/button.hh"

namespace zod {

static auto grid = true;
static auto cursor_position = 0.0f;

constexpr auto padding = 4.0f;
constexpr auto inner = 4.0f;

static auto Button(const char* name, bool& enabled) -> void {
  ImGui::SetNextItemAllowOverlap();
  auto button_size = ImGui::CalcTextSize(name);
  cursor_position = ImGui::GetWindowContentRegionMax().x - button_size.x -
                    padding - inner * 2 - cursor_position;
  auto cursor =
      ImVec2(cursor_position, ImGui::GetWindowContentRegionMin().y + padding);
  cursor_position = button_size.x + padding + inner * 2;
  ImGui::SetCursorPos(cursor);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(inner, inner));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.f);
  if (enabled) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0.45f, 0.82f, 1));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0.45f, 0.82f, 1));
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

Viewport::Viewport()
    : SPanel("Viewport",
             shared<PerspectiveCamera>(64, 64, 90.0f, 0.01f, 100.0f), false),
      m_width(64), m_height(64) {
  m_framebuffer = Editor::get().get_renderer().get_render_target();
  m_framebuffer->bind();
  GPUAttachment attach = { GPUBackend::get().create_texture({
      .width = i32(m_width),
      .height = i32(m_height),
  }) };
  m_framebuffer->add_color_attachment(attach);
  m_framebuffer->add_depth_attachment();
  m_framebuffer->check();
  m_framebuffer->unbind();

  // Shader taken from
  // https://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
  m_grid_shader =
      GPUBackend::get().create_shader(GPUShaderCreateInfo("grid")
                                          .vertex_source(g_grid_vert_src)
                                          .fragment_source(g_grid_frag_src));
}

auto Viewport::draw_grid() -> void {
  m_grid_shader->bind();
  m_grid_shader->uniform_float("u_color", value_ptr(vec3(0.25f)), 3);
  GPUState::get().draw_immediate(6);
}

auto Viewport::on_event_imp(Event&) -> void {}

auto Viewport::draw_imp(Geometry&) -> void {
  Editor::get().get_renderer().tick();
  auto texture = m_framebuffer->get_slot(0).texture;
  ImGui::Image(texture->get_id(), ImVec2(m_size.x, m_size.y),
               ImVec2 { 0.0, 0.0 }, ImVec2 { 1.0, -1.0 });
  Button("Grid", grid);
  cursor_position = 0.0f;
}

} // namespace zod
