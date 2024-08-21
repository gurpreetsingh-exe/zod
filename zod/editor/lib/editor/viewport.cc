#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "application/context.hh"
#include "viewport.hh"

namespace zod {

static auto grid = true;
constexpr auto padding = 4.0f;
constexpr auto inner = 4.0f;

struct CameraUBO {
  glm::mat4 view_projection;
  glm::vec4 direction;
};

Viewport::Viewport()
    : m_width(64), m_height(64),
      m_camera(Camera(64, 64, 90.0f, 0.01f, 100.0f)) {
  m_framebuffer = GPUBackend::get().create_framebuffer(m_width, m_height);
  m_framebuffer->bind();
  GPUAttachment attach = { GPUBackend::get().create_texture(
      GPUTextureType::Texture2D, m_width, m_height, false) };
  m_framebuffer->add_color_attachment(attach);
  m_framebuffer->add_depth_attachment();
  m_framebuffer->check();
  m_framebuffer->unbind();

  m_shader = GPUBackend::get().create_shader("view_3d");
  m_shader->init_vertex_shader(g_view3d_vert);
  m_shader->init_fragment_shader(g_view3d_frag);
  m_shader->compile();

  f32 position[] = {
    -1, -1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1,
    -1, -1, 1,  1, -1, 1,  1, 1, 1,  -1, 1, 1,
  };

  auto format = std::vector<GPUBufferLayout> {
    { GPUDataType::Float, position, 3, 24 },
  };
  m_cubemap_batch = GPUBackend::get().create_batch(
      format, { 1, 2, 0, 2, 3, 0, 6, 2, 1, 1, 5, 6, 6, 5, 4, 4, 7, 6,
                6, 3, 2, 7, 3, 6, 3, 7, 0, 7, 4, 0, 5, 1, 0, 4, 5, 0 });

  m_cubemap_shader = GPUBackend::get().create_shader("cubemap");
  m_cubemap_shader->init_vertex_shader(g_cubemap_vert);
  m_cubemap_shader->init_fragment_shader(g_cubemap_frag);
  m_cubemap_shader->compile();

  m_camera_ubo = GPUBackend::get().create_uniform_buffer(sizeof(CameraUBO));

  // Shader taken from
  // https://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
  m_grid_shader = GPUBackend::get().create_shader("grid");
  m_grid_shader->init_vertex_shader(g_grid_vert);
  m_grid_shader->init_fragment_shader(g_grid_frag);
  m_grid_shader->compile();
}

auto Viewport::draw_cubemap() -> void {
  glDepthFunc(GL_LEQUAL);
  m_cubemap_shader->bind();
  m_cubemap_batch->draw(m_cubemap_shader);
  glDepthFunc(GL_LESS);
}

auto Viewport::draw_grid() -> void {
  glBindVertexArray(0);
  m_grid_shader->bind();
  m_grid_shader->uniform("u_color", glm::vec3(0.25f));
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

auto Viewport::draw_axes() -> void {}

static auto Button(const char* name, bool& enabled) -> void {
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

auto Viewport::update(Shared<GPUBatch> batch) -> void {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("Viewport");

  auto update_camera_ubo = [&] {
    auto ubo = CameraUBO { m_camera.get_view_projection(),
                           glm::vec4(m_camera.get_direction(), 0.0f) };
    m_camera_ubo->upload_data(&ubo, sizeof(CameraUBO));
  };

  auto position = ImGui::GetWindowPos();
  m_camera.set_window_position(glm::vec2(position.x, position.y));

  auto size = ImGui::GetContentRegionAvail();
  if (size.x != m_width or size.y != m_height) {
    m_width = size.x;
    m_height = size.y;
    m_framebuffer->resize(m_width, m_height);
    m_camera.resize(m_width, m_height);
  }

  m_camera.update();
  m_camera.updating = ImGui::IsWindowHovered();
  update_camera_ubo();

  m_framebuffer->bind();
  m_camera_ubo->bind();
  m_framebuffer->clear();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  GPU_TIME("mesh", {
    m_shader->bind();
    batch->draw(m_shader);
  });
  GPU_TIME("cubemap", { draw_cubemap(); });
  if (grid) {
    GPU_TIME("grid", { draw_grid(); });
  }
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  m_framebuffer->unbind();

  auto& texture = m_framebuffer->get_slot(0).texture;
  ImGui::Image(texture->get_id(), size, ImVec2 { 0.0, 0.0 },
               ImVec2 { 1.0, -1.0 });
  Button("Grid", grid);

  ImGui::End();
  ImGui::PopStyleVar();
}

} // namespace zod
