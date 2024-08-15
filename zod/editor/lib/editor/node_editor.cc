#include <imgui.h>

#include "input.hh"
#include "node_editor.hh"

namespace zod {

struct CameraUBO {
  glm::mat4 view_projection;
  glm::vec4 direction;
};

NodeEditor::NodeEditor()
    : m_width(64), m_height(64), m_camera(OrthographicCamera(64.0f, 64.0f)) {
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

  m_node_shader = GPUBackend::get().create_shader("node");
  m_node_shader->init_vertex_shader(g_node_vert);
  m_node_shader->init_fragment_shader(g_uv);
  m_node_shader->compile();

  f32 position[] = { 0, 0, 1, 0, 0, 1, 1, 1 };
  auto format = std::vector<GPUBufferLayout> {
    { GPUDataType::Float, position, 2, 8 },
  };

  m_batch = GPUBackend::get().create_batch(format, { 1, 2, 0, 2, 3, 1 });

  m_camera_ubo = GPUBackend::get().create_uniform_buffer(sizeof(CameraUBO));
  m_node_locations = GPUBackend::get().create_storage_buffer();

  glm::vec2 loc[2] = { { 0, 0 }, { 100, 0 } };
  m_node_locations->upload_data(loc, 2 * sizeof(glm::vec2));
}

auto NodeEditor::add_node() -> void { m_node_add = true; }

auto NodeEditor::update() -> void {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("NodeEditor");

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
  auto ubo = CameraUBO { m_camera.get_view_projection(), glm::vec4(0.0f) };
  m_camera_ubo->upload_data(&ubo, sizeof(CameraUBO));

  ImGui::PopStyleVar();

  if (ImGui::BeginPopupContextWindow("Add Menu",
                                     ImGuiPopupFlags_MouseButtonRight)) {
    if (ImGui::MenuItem("Cube")) {
      add_node();
    }

    ImGui::EndPopup();
  }

  if (m_node_add) {
    if (Input::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
      m_node_add = false;
    }

    if (Input::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT)) {
      m_node_add = false;
    }
  }

  m_camera_ubo->bind(1);
  m_framebuffer->bind();
  m_framebuffer->clear();
  // glEnable(GL_DEPTH_TEST);
  glClearColor(0, 0, 0, 1);
  m_shader->bind();
  glBindVertexArray(0);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  m_node_locations->bind(1);
  m_node_shader->bind();
  m_batch->draw_instanced(m_node_shader, 2);
  // glDisable(GL_DEPTH_TEST);
  m_framebuffer->unbind();

  auto& texture = m_framebuffer->get_slot(0).texture;
  ImGui::Image(texture->get_id(), size, ImVec2 { 0.0, 0.0 },
               ImVec2 { 1.0, -1.0 });

  ImGui::End();
}

} // namespace zod
