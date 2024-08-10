#include <imgui.h>

#include "input.hh"
#include "node_editor.hh"

namespace zod {

NodeEditor::NodeEditor()
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

  m_shader = GPUBackend::get().create_shader("node_editor");
  m_shader->init_vertex_shader(g_fullscreen);
  m_shader->init_fragment_shader(g_node_editor_frag);
  m_shader->compile();

  f32 position[] = { -1, -1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1,
                     -1, -1, 1,  1, -1, 1,  1, 1, 1,  -1, 1, 1 };
  auto format = std::vector<GPUBufferLayout> {
    { GPUDataType::Float, position, 3, 24 },
  };

  m_batch = GPUBackend::get().create_batch(
      format, { 1, 2, 0, 2, 3, 0, 6, 2, 1, 1, 5, 6, 6, 5, 4, 4, 7, 6,
                6, 3, 2, 7, 3, 6, 3, 7, 0, 7, 4, 0, 5, 1, 0, 4, 5, 0 });
}

auto NodeEditor::update() -> void {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("NodeEditor");

  if (ImGui::IsWindowHovered()) {
    m_camera.update();
  }

  auto size = ImGui::GetContentRegionAvail();
  if (size.x != m_width or size.y != m_height) {
    m_width = size.x;
    m_height = size.y;
    m_framebuffer->resize(m_width, m_height);
    m_camera.resize(m_width, m_height);
    m_camera.update();
  }

  ImGui::PopStyleVar();

  if (ImGui::BeginPopupContextWindow("Add Menu",
                                     ImGuiPopupFlags_MouseButtonRight)) {
    if (ImGui::MenuItem("Empty")) {}

    ImGui::EndPopup();
  }

  m_framebuffer->bind();
  m_framebuffer->clear();
  // glEnable(GL_DEPTH_TEST);
  glClearColor(0, 0, 0, 1);
  m_shader->bind();
  m_shader->uniform("u_width", m_width);
  m_shader->uniform("u_height", m_height);
  glBindVertexArray(0);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  // m_shader->uniform("u_view_projection", m_camera.get_view_projection());
  // m_batch->draw(m_shader);
  // glDisable(GL_DEPTH_TEST);
  m_framebuffer->unbind();

  auto& texture = m_framebuffer->get_slot(0).texture;
  ImGui::Image(texture->get_id(), size, ImVec2 { 0.0, 0.0 },
               ImVec2 { 1.0, -1.0 });

  ImGui::End();
}

} // namespace zod