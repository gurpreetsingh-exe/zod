#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "input.hh"
#include "node_editor.hh"
#include "widgets/button.hh"

#include "curve.hh"

namespace zod {

NodeEditor::NodeEditor()
    : Panel("Node Editor", unique<OrthographicCamera>(64.0f, 64.0f)),
      m_width(64), m_height(64), m_node_tree(shared<NodeTree>()) {
  m_font = unique<Font>();
  m_font->load_font("../third-party/imgui/misc/fonts/DroidSans.ttf");
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

  m_line_shader = GPUBackend::get().create_shader("line");
  m_line_shader->init_vertex_shader(g_vertex2d);
  m_line_shader->init_fragment_shader(g_flat_color_frag);
  m_line_shader->compile();

  f32 position[] = { 0, 0, 1, 0, 0, 1, 1, 1 };
  auto format = std::vector<GPUBufferLayout> {
    { GPUDataType::Float, position, 2, 8 },
  };

  m_batch = GPUBackend::get().create_batch(format, { 1, 2, 0, 2, 3, 1 });

  {
    // clang-format off
    auto curve = RoundCurve(
      vec2(-100, 100),
      vec2(-200, 400),
      40.0f
    );
    // clang-format on

    auto position = curve.get();
    auto buf = std::vector<f32>();
    for (usize i = 0; i < position.size() - 1; ++i) {
      auto p1 = position[i];
      auto p2 = position[i + 1];
      buf.push_back(p1.x);
      buf.push_back(p1.y);
      buf.push_back(p2.x);
      buf.push_back(p2.y);
    }

    auto format = std::vector<GPUBufferLayout> {
      { GPUDataType::Float, buf.data(), 2, buf.size() },
    };
    m_curves = GPUBackend::get().create_batch(format);
  }

  for (usize i = 0; i < 5; ++i) {
    m_node_tree->add_node<NODE_TRANSFORM>(vec2(-100, (f32(i) * 150) - 400));
  }

  m_node_ssbo = GPUBackend::get().create_storage_buffer();

  m_node_ssbo->upload_data(m_node_tree->get_data(),
                           m_node_tree->get_size() * sizeof(NodeType));
}

auto NodeEditor::add_node(usize type, vec2 position) -> void {
  m_node_add = true;
  auto pos = vec2(m_camera->screen_to_world(position)) - vec2(NODE_SIZE);
  auto& node = m_node_tree->add_node(type, pos);
  m_active = node.type->id;
  m_node_ssbo->upload_data(m_node_tree->get_data(),
                           m_node_tree->get_size() * sizeof(NodeType));
}

auto NodeEditor::draw_props() -> void {
  if (not m_active) {
    return;
  }
  auto* node = m_node_tree->node_from_id(m_active);
  auto name =
      fmt::format("{}.{}", node_names[node->type->type], node->type->id);
  ImGui::SeparatorText(name.c_str());
  ImGui::Spacing();
  for (usize i = 0; i < node->props.size(); ++i) {
    auto& prop = node->props[i];
    if (prop.draw() and m_node_tree->get_visualized() == node->type->id) {
      node->update(*node);
    }
  }
  // for (auto& prop : node->props) { prop.draw(); }
  // node->draw(*node);
}

auto NodeEditor::update() -> void {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("NodeEditor");

  auto position = ImGui::GetWindowPos();
  m_camera->set_window_position(vec2(position.x, position.y));

  auto mouse_pos = ImGui::GetMousePos() - position - ImVec2(0, 20);
  auto pos = vec2(mouse_pos.x, mouse_pos.y);
  auto delta = vec2(m_camera->screen_to_world(pos)) -
               vec2(m_camera->screen_to_world(m_last_mouse_pos));

  auto update_camera = [&] {
    m_camera->update();
    m_camera->force_update(ImGui::IsWindowHovered());
    auto storage = CameraUniformBufferStorage { m_camera->get_view_projection(),
                                                vec4(0.0f) };
    m_uniform_buffer->upload_data(&storage, sizeof(CameraUniformBufferStorage));
  };

  auto size = ImGui::GetContentRegionAvail();
  if (size.x != m_width or size.y != m_height) {
    m_width = size.x;
    m_height = size.y;
    m_framebuffer->resize(m_width, m_height);
    m_camera->resize(m_width, m_height);
    update_camera();
  }

  auto is_camera_updating = Input::is_key_pressed(GLFW_KEY_RIGHT_ALT);
  if (is_camera_updating) {
    update_camera();
  }

  ImGui::PopStyleVar();

  if (ImGui::BeginPopupContextWindow("Add Menu",
                                     ImGuiPopupFlags_MouseButtonRight)) {
    for (usize i = 1; i < TOTAL_NODES; ++i) {
      if (ImGui::MenuItem(node_names[i])) {
        add_node(i, pos);
      }
    }

    ImGui::EndPopup();
  }

  if (m_node_add) {
    update_node([&](auto* node) { node->type->location += delta; });

    if (Input::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
      m_node_add = false;
    }

    if (Input::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT)) {
      m_node_add = false;
      m_active = 0;
    }
  }

  m_uniform_buffer->bind(1);
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
  m_node_shader->uniform("u_vis", m_node_tree->get_visualized());
  m_batch->draw_instanced(m_node_shader, m_node_tree->get_size());

  m_line_shader->bind();
  m_line_shader->uniform("u_color", vec3(1.0f));
  m_curves->draw_lines(m_line_shader);

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

  auto click = delta.x == 0 and delta.y == 0;
  auto pixel = 0u;
  if (ImGui::IsWindowHovered() and not is_camera_updating and
      Input::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
    if (not click and m_active) {
      update_node([&](auto* node) { node->type->location += delta; });
    }
    pixel = pos.x > m_framebuffer->get_width() or
                    pos.y > m_framebuffer->get_height()
                ? 0
                : m_framebuffer->read_pixel(
                      1, pos.x, m_framebuffer->get_height() - pos.y);
    if (auto id = pixel & 0xffffff) {
      auto extra = (pixel & 0xff000000) >> 24;
      struct Mask {
        union {
          struct {
            u8 visualize : 1;
            u8 padding : 7;
          };
          u8 data;
        };
        Mask(u8 d) : data(d) {}
      };
      static_assert(sizeof(Mask) == 1);
      auto mask = Mask(extra);
      switch (mask.visualize) {
        case 0:
          break;
        case 1: {
          m_node_tree->set_visualized(id);
          auto* node = m_node_tree->node_from_id(id);
          node->update(*node);
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

  ImGui::SetCursorPos(ImVec2(5, 20));
  ImGui::Text("Active: %u", m_active);
  ImGui::SetCursorPos(ImVec2(5, 35));
  ImGui::Text("Pixel: %u (%u, %u, %u, %u)", pixel, (pixel & 0x000000ff),
              (pixel & 0x0000ff00) >> 8, (pixel & 0x00ff0000) >> 16,
              (pixel & 0xff000000) >> 24);
  ImGui::SetCursorPos(ImVec2(5, 50));

  ImGui::End();

  m_last_mouse_pos = pos;
}

} // namespace zod
