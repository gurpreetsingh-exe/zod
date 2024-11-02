#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "input.hh"
#include "node_editor.hh"
#include "widgets/button.hh"

#include "curve.hh"

namespace zod {

enum class AddMode {
  None = 0,
  Node,
  Link,
};

static AddMode g_add_mode = AddMode::None;

NodeEditor::NodeEditor()
    : Panel("Node Editor", unique<OrthographicCamera>(64.0f, 64.0f), false),
      m_width(64), m_height(64) {
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

  m_shader =
      GPUBackend::get().create_shader(GPUShaderCreateInfo("node_editor")
                                          .vertex_source(g_fullscreen)
                                          .fragment_source(g_node_editor_frag));

  m_node_shader =
      GPUBackend::get().create_shader(GPUShaderCreateInfo("node")
                                          .vertex_source(g_node_vert)
                                          .fragment_source(g_node_frag));

  m_line_shader =
      GPUBackend::get().create_shader(GPUShaderCreateInfo("line")
                                          .vertex_source(g_vertex2d)
                                          .fragment_source(g_flat_color_frag));

  f32 position[] = { 0, 0, 1, 0, 0, 1, 1, 1 };
  auto format = std::vector<GPUBufferLayout> {
    { GPUDataType::Float, position, 2, 8 },
  };

  m_batch = GPUBackend::get().create_batch(format, { 1, 2, 0, 2, 3, 1 });

  m_curves = GPUBackend::get().create_batch(
      { { GPUDataType::Float, m_links.data(), 2, 0 } });

  auto node_tree = ZCtxt::get().get_node_tree();
  for (usize i = 0; i < 5; ++i) {
    node_tree->add_node<NODE_TRANSFORM>(vec2(-100, (f32(i) * 150) - 400));
  }

  m_node_ssbo = GPUBackend::get().create_storage_buffer();

  m_node_ssbo->upload_data(node_tree->get_data(),
                           node_tree->get_size() * sizeof(NodeType));
}

auto NodeEditor::add_node(usize type) -> void {
  auto position = Input::get_mouse_pos() - m_position + vec2(0, 43);
  g_add_mode = AddMode::Node;
  auto pos = vec2(m_camera->screen_to_world(position)) - vec2(NODE_SIZE);
  auto node_tree = ZCtxt::get().get_node_tree();
  auto& node = node_tree->add_node(type, pos);
  m_node_ssbo->upload_data(node_tree->get_data(),
                           node_tree->get_size() * sizeof(NodeType));
}

auto NodeEditor::on_event_imp(Event& event) -> void {
  switch (event.kind) {
    case Event::MouseDown: {
      auto node_tree = ZCtxt::get().get_node_tree();
      auto pos = Input::get_mouse_pos() - m_position + vec2(0, 43);
      auto delete_link = [&] {
        m_links.pop_back();
        m_links.pop_back();
        m_curves->update_binding(0, m_links.data(),
                                 sizeof(vec2) * m_links.size());
      };
      if (g_add_mode == AddMode::Node) {
        g_add_mode = AddMode::None;
        if (event.button == Event::MouseButtonLeft) {
        } else if (event.button == Event::MouseButtonRight) {
          node_tree->set_active_id(0);
        }
        return;
      } else if (g_add_mode == AddMode::Link) {
        if (event.button == Event::MouseButtonLeft) {
          auto& second = m_links.back();
          second = m_camera->screen_to_world(pos);
          m_curves->update_binding(0, m_links.data(),
                                   sizeof(vec2) * m_links.size());
        } else if (event.button == Event::MouseButtonRight) {
          g_add_mode = AddMode::None;
          delete_link();
          return;
        }
      }

      auto pixel = 0u;
      pixel = pos.x > m_framebuffer->get_width() or
                      pos.y > m_framebuffer->get_height()
                  ? 0
                  : m_framebuffer->read_pixel(
                        1, pos.x, m_framebuffer->get_height() - pos.y);
      m_debug_message =
          fmt::format("Pixel: {}\n{}, {}, {}, {}", pixel, (pixel & 0x000000ff),
                      (pixel & 0x0000ff00) >> 8, (pixel & 0x00ff0000) >> 16,
                      (pixel & 0xff000000) >> 24);
      if (auto id = pixel & 0xffffff) {
        auto extra = (pixel & 0xff000000) >> 24;
        struct Mask {
          union {
            struct {
              u8 visualize : 1;
              u8 socket_in : 1;
              u8 socket_out : 1;
              u8 padding : 5;
            };
            u8 data;
          };
          Mask(u8 d) : data(d) {}
        };
        static_assert(sizeof(Mask) == 1);
        auto mask = Mask(extra);
        if (mask.visualize) {
          node_tree->set_visualized(id);
          auto* node = node_tree->node_from_id(id);
          node->update(*node);
          node_tree->set_active_id(id);
        } else if (mask.socket_in) {
          if (g_add_mode == AddMode::None) {
            g_add_mode = AddMode::Link;
          } else {
            auto* node = node_tree->node_from_id(id);
            g_add_mode = AddMode::None;
            auto& second = m_links.back();
            second = node->type->location + vec2(NODE_SIZE) + vec2(0, 38);
            m_curves->update_binding(0, m_links.data(),
                                     sizeof(vec2) * m_links.size());
          }
        } else if (mask.socket_out) {
          auto* node = node_tree->node_from_id(id);
          g_add_mode = AddMode::Link;
          auto p = node->type->location + vec2(NODE_SIZE) - vec2(0, 38);
          m_links.push_back(p);
          m_links.push_back(m_camera->screen_to_world(pos));
          m_curves->get_buffer(0)->upload_data(m_links.data(),
                                               sizeof(vec2) * m_links.size());
        } else {
          if (g_add_mode == AddMode::Link) {
            g_add_mode = AddMode::None;
            delete_link();
            return;
          }
          node_tree->set_active_id(id);
        }
      } else {
        if (g_add_mode == AddMode::Link) {
          g_add_mode = AddMode::None;
          delete_link();
          return;
        }
        node_tree->set_active_id(0);
      }
    } break;
    case Event::MouseMove: {
      auto pos = event.mouse;
      auto delta = vec2(m_camera->screen_to_world(pos)) -
                   vec2(m_camera->screen_to_world(g_last_mouse_pos));
      if (event.button == Event::MouseButtonLeft or
          g_add_mode == AddMode::Node) {
        update_node([&](auto* node) { node->type->location += delta; });
      }

      if (g_add_mode == AddMode::Link) {
        auto& second = m_links.back();
        second += delta;
        m_curves->update_binding(0, m_links.data(),
                                 sizeof(vec2) * m_links.size());
      }
    } break;
    case Event::KeyDown: {
      if (event.key == Key::Tab) {
        m_framebuffer_bit = not m_framebuffer_bit;
      }
    } break;
  }
}

auto NodeEditor::update() -> void {
  auto node_tree = ZCtxt::get().get_node_tree();
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::PopStyleVar();
  if (ImGui::BeginPopupContextWindow("Add Menu",
                                     ImGuiPopupFlags_MouseButtonRight)) {
    for (usize i = 1; i < TOTAL_NODES; ++i) {
      if (ImGui::MenuItem(node_names[i])) {
        add_node(i);
      }
    }

    ImGui::EndPopup();
  }

  m_uniform_buffer->bind(1);
  m_framebuffer->bind();
  m_framebuffer->clear();
  GPUState::get().set_blend(Blend::Alpha);
  m_shader->bind();
  GPUState::get().draw_immediate(3);
  m_node_ssbo->bind(1);
  m_node_shader->bind();
  m_node_shader->uniform_uint("u_active",
                              ADDR(u32(node_tree->get_active_id())));
  m_node_shader->uniform_uint("u_vis", ADDR(u32(node_tree->get_visualized())));
  m_batch->draw_instanced(m_node_shader, node_tree->get_size());

  m_line_shader->bind();
  m_line_shader->uniform_float("u_color", ADDROF(vec3(1.0f)), 3);
  m_curves->draw_lines(m_line_shader, m_links.size());

  for (const auto& node : node_tree->get_nodes()) {
    auto loc = node.type->location;
    m_font->render_text(node_names[node.type->type], loc.x + 200,
                        loc.y + 100 - (Font::size >> 2), 1, 1);
  }
  m_font->submit();
  GPUState::get().set_blend(Blend::None);
  m_framebuffer->unbind();

  auto& texture = m_framebuffer->get_slot(m_framebuffer_bit).texture;
  ImGui::Image(texture->get_id(), ImVec2(m_size.x, m_size.y),
               ImVec2 { 0.0, 0.0 }, ImVec2 { 1.0, -1.0 });
}

} // namespace zod
