#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "input.hh"
#include "node_editor.hh"
#include "widgets/button.hh"

namespace zod {

struct CameraUBO {
  mat4 view_projection;
  vec4 direction;
};

NodeEditor::NodeEditor()
    : m_width(64), m_height(64), m_camera(OrthographicCamera(64.0f, 64.0f)),
      m_node_tree(shared<NodeTree>()) {
  m_font = unique<Font>();
  m_font->load_font("../third-party/imgui/misc/fonts/DroidSans.ttf");
  m_framebuffer = GPUBackend::get().create_framebuffer(m_width, m_height);
  m_framebuffer->bind();
  GPUAttachment attach = { GPUBackend::get().create_texture(
      GPUTextureType::Texture2D, GPUTextureFormat::RGBA8, m_width, m_height,
      false) };
  m_framebuffer->add_color_attachment(attach);
  attach = { GPUBackend::get().create_texture(GPUTextureType::Texture2D,
                                              GPUTextureFormat::R32UI, m_width,
                                              m_height, false) };
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
  m_node_shader->init_fragment_shader(g_node_frag);
  m_node_shader->compile();

  f32 position[] = { 0, 0, 1, 0, 0, 1, 1, 1 };
  auto format = std::vector<GPUBufferLayout> {
    { GPUDataType::Float, position, 2, 8 },
  };

  m_batch = GPUBackend::get().create_batch(format, { 1, 2, 0, 2, 3, 1 });

  for (usize i = 0; i < 5; ++i) {
    m_node_tree->add_node<NODE_TRANSFORM>(vec2(-100, (f32(i) * 150) - 400));
  }

  m_camera_ubo = GPUBackend::get().create_uniform_buffer(sizeof(CameraUBO));
  m_node_ssbo = GPUBackend::get().create_storage_buffer();

  m_node_ssbo->upload_data(m_node_tree->get_data(),
                           m_node_tree->get_size() * sizeof(NodeType));
}

auto NodeEditor::add_node() -> void { m_node_add = true; }

auto NodeEditor::draw_props() -> void {
  if (not m_active) {
    return;
  }
  auto* node = m_node_tree->node_from_id(m_active);
  auto name =
      fmt::format("{}.{}", node_names[node->type->type], node->type->id);
  ImGui::SeparatorText(name.c_str());
  ImGui::Spacing();
  for (auto& prop : node->props) { prop.draw(); }
  // node->draw(*node);
}

auto NodeEditor::update() -> void {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("NodeEditor");

  auto position = ImGui::GetWindowPos();
  m_camera.set_window_position(vec2(position.x, position.y));

  auto update_camera = [&] {
    m_camera.update();
    m_camera.updating = ImGui::IsWindowHovered();
    auto ubo = CameraUBO { m_camera.get_view_projection(), vec4(0.0f) };
    m_camera_ubo->upload_data(&ubo, sizeof(CameraUBO));
  };

  auto size = ImGui::GetContentRegionAvail();
  if (size.x != m_width or size.y != m_height) {
    m_width = size.x;
    m_height = size.y;
    m_framebuffer->resize(m_width, m_height);
    m_camera.resize(m_width, m_height);
    update_camera();
  }

  auto is_camera_updating = Input::is_key_pressed(GLFW_KEY_RIGHT_ALT);
  if (is_camera_updating) {
    update_camera();
  }

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
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  m_shader->bind();
  glBindVertexArray(0);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  m_node_ssbo->bind(1);
  m_node_shader->bind();
  m_node_shader->uniform("u_active", m_active);
  m_node_shader->uniform("u_vis", m_vis);
  m_batch->draw_instanced(m_node_shader, m_node_tree->get_size());
  for (const auto& node : m_node_tree->get_nodes()) {
    auto loc = node.type->location;
    m_font->render_text(node_names[node.type->type], loc.x + 200,
                        loc.y + 100 - (Font::size >> 2), 1, 1);
  }
  m_font->submit();
  glDisable(GL_BLEND);
  m_framebuffer->unbind();

  auto& texture = m_framebuffer->get_slot(0).texture;
  ImGui::Image(texture->get_id(), size, ImVec2 { 0.0, 0.0 },
               ImVec2 { 1.0, -1.0 });

  auto mouse_pos = ImGui::GetMousePos() - position - ImVec2(0, 20);
  auto pos = vec2(mouse_pos.x, mouse_pos.y);
  auto delta = pos - m_last_mouse_pos;
  auto click = delta.x == 0 and delta.y == 0;
  auto pixel = 0u;
  if (ImGui::IsWindowHovered() and not is_camera_updating and
      Input::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
    if (not click and m_active) {
      m_node_tree->update_node(m_active, [&](auto* node) {
        delta /= m_camera.get_zoom();
        node->type->location.x += delta.x;
        node->type->location.y -= delta.y;
        m_node_ssbo->update_data(node->type, sizeof(NodeType),
                                 (m_active - 1) * sizeof(NodeType));
      });
    }
    pixel = pos.x > m_framebuffer->get_width() or
                    pos.y > m_framebuffer->get_height()
                ? 0
                : m_framebuffer->read_pixel(
                      1, pos.x, m_framebuffer->get_height() - pos.y);
    if (auto id = pixel & 0xffffff) {
      auto extra = (pixel & 0xff000000) >> 24;
      switch (extra) {
        case 0:
          break;
        case 1: {
          m_vis = id;
          // if (m_active) {
          //   node = &m_node_tree->get_data()[m_active - 1];
          //   node->extra = 0;
          //   m_node_locations->update_data(node, sizeof(Node),
          //                                 (m_active - 1) * sizeof(Node));
          // }
        } break;
        default:
          UNREACHABLE();
          break;
      }
      if (click) {
        m_active = id;
      }
    } else if (click) {
      m_active = 0;
    }
  }

  m_last_mouse_pos = pos;

  ImGui::SetCursorPos(ImVec2(5, 20));
  ImGui::Text("Active: %u", m_active);
  ImGui::SetCursorPos(ImVec2(5, 35));
  ImGui::Text("Pixel: %u (%u, %u, %u, %u)", pixel, (pixel & 0x000000ff),
              (pixel & 0x0000ff00) >> 8, (pixel & 0x00ff0000) >> 16,
              (pixel & 0xff000000) >> 24);
  ImGui::SetCursorPos(ImVec2(5, 50));
  ImGui::Text("Zoom: %f", m_camera.get_zoom());

  ImGui::End();
}

} // namespace zod
