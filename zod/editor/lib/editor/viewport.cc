#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "editor.hh"
#include "engine/camera.hh"
#include "engine/runtime.hh"
#include "gpu/timer.hh"
#include "viewport.hh"
#include "widgets/button.hh"

#include "gpu/shader_builtins.hh"

namespace zod {

static auto grid = false;
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

  m_icons = GPUBackend::get().create_texture(
      { .path = "./lighting-bulb-1.png", .mips = 4 });
  m_icons->generate_mipmap();

  // Shader taken from
  // https://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
  GPUBackend::get().create_shader(GPUShaderCreateInfo("grid")
                                      .vertex_source(g_grid_vert_src)
                                      .fragment_source(g_grid_frag_src));

  GPUBackend::get().create_shader(GPUShaderCreateInfo("billboard")
                                      .vertex_source(g_billboard_vert_src)
                                      .fragment_source(g_texture_src));

  m_overlay_batch = GPUBackend::get().create_batch({}, { 1, 2, 0, 2, 3, 1 });
}

auto Viewport::draw_overlays() -> void {
  m_framebuffer->bind();
  GPUState::get().set_depth_test(Depth::Less);
  GPUState::get().set_blend(Blend::Alpha);
  if (grid) {
    // grid
    auto shader = GPUBackend::get().get_shader("grid");
    shader->bind();
    shader->uniform_float("u_color", ADDROF(vec3(0.25f)), 3);
    GPUState::get().draw_immediate(6);
  }
  {
    // icons
    auto shader = GPUBackend::get().get_shader("billboard");
    shader->bind();
    m_icons->bind();
    shader->uniform_int("u_texture", ADDR(0));
    auto lights = Runtime::get().scene().stats().number_lights;
    m_overlay_batch->draw_instanced(shader, lights);
  }
  GPUState::get().set_depth_test(Depth::None);
  GPUState::get().set_blend(Blend::None);
  m_framebuffer->unbind();
}

auto Viewport::on_event_imp(Event& event) -> void {
  if (event.kind == Event::WindowResize) {
    Editor::get().get_renderer().resize(event.size.x, event.size.y);
  }
}

auto Viewport::draw_imp(Geometry&) -> void {
  Editor::get().get_renderer().tick();
  draw_overlays();

  auto texture = m_framebuffer->get_slot(0);
  ImGui::Image(texture->get_id(), ImVec2(m_size.x, m_size.y),
               ImVec2 { 0.0, 0.0 }, ImVec2 { 1.0, -1.0 });
  Button("Grid", grid);
  cursor_position = 0.0f;
}

} // namespace zod
