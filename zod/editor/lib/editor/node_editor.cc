#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "input.hh"
#include "node_editor.hh"
#include "widgets/button.hh"

#include "curve.hh"

namespace zod {

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
  position.y = m_size.y - position.y;
  m_node_add = true;
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
      if (m_node_add) {
        if (event.button == Event::MouseButtonLeft) {
          m_node_add = false;
        } else if (event.button == Event::MouseButtonRight) {
          m_node_add = false;
          node_tree->set_active_id(0);
        }
        return;
      }

      auto pos = Input::get_mouse_pos() - m_position + vec2(0, 43);
      auto pixel = 0u;
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
            node_tree->set_visualized(id);
            auto* node = node_tree->node_from_id(id);
            node->update(*node);
          } break;
          default:
            UNREACHABLE();
            break;
        }
        node_tree->set_active_id(id);
      } else {
        node_tree->set_active_id(0);
      }
    } break;
    case Event::MouseMove: {
      auto pos = event.mouse;
      auto delta = vec2(m_camera->screen_to_world(pos)) -
                   vec2(m_camera->screen_to_world(g_last_mouse_pos));

      if (event.button == Event::MouseButtonLeft or m_node_add) {
        update_node([&](auto* node) {
          node->type->location += vec2(delta.x, -delta.y);
        });
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
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  m_shader->bind();
  glBindVertexArray(0);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  m_node_ssbo->bind(1);
  m_node_shader->bind();
  m_node_shader->uniform("u_active", node_tree->get_active_id());
  m_node_shader->uniform("u_vis", node_tree->get_visualized());
  m_batch->draw_instanced(m_node_shader, node_tree->get_size());

  m_line_shader->bind();
  m_line_shader->uniform("u_color", vec3(1.0f));
  m_curves->draw_lines(m_line_shader);

  for (const auto& node : node_tree->get_nodes()) {
    auto loc = node.type->location;
    m_font->render_text(node_names[node.type->type], loc.x + 200,
                        loc.y + 100 - (Font::size >> 2), 1, 1);
  }
  m_font->submit();
  glDisable(GL_BLEND);
  m_framebuffer->unbind();

  auto& texture = m_framebuffer->get_slot(0).texture;
  ImGui::Image(texture->get_id(), ImVec2(m_size.x, m_size.y),
               ImVec2 { 0.0, 0.0 }, ImVec2 { 1.0, -1.0 });
}

} // namespace zod
