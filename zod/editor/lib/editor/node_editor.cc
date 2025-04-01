#include <imgui.h>

#include "application/input.hh"
#include "curve.hh"
#include "engine/camera.hh"
#include "engine/font.hh"
#include "node_editor.hh"
#include "nodes.hh"
#include "operators/node.hh"
#include "widgets/button.hh"

namespace zod {

NodeEditor::NodeEditor()
    : SPanel("Node Editor", shared<OrthographicCamera>(64.0f, 64.0f), false),
      m_width(64), m_height(64) {
  m_framebuffer->bind();
  GPUAttachment attach = { GPUBackend::get().create_texture({
      .width = i32(m_width),
      .height = i32(m_height),
  }) };
  m_framebuffer->add_color_attachment(attach);
  attach = { GPUBackend::get().create_texture({
      .width = i32(m_width),
      .height = i32(m_height),
      .format = GPUTextureFormat::R32UI,
  }) };
  m_framebuffer->add_color_attachment(attach);
  m_framebuffer->add_depth_attachment();
  m_framebuffer->check();
  m_framebuffer->unbind();

  m_shader = GPUBackend::get().create_shader(
      GPUShaderCreateInfo("node_editor")
          .vertex_source(g_fullscreen_src)
          .fragment_source(g_node_editor_frag_src));
  m_node_shader =
      GPUBackend::get().create_shader(GPUShaderCreateInfo("node")
                                          .vertex_source(g_node_vert_src)
                                          .fragment_source(g_node_frag_src));
  m_line_shader = GPUBackend::get().create_shader(
      GPUShaderCreateInfo("line")
          .vertex_source(g_view2d_vert_src)
          .fragment_source(g_flat_color_frag_src));

  f32 position[] = { 0, 0, 1, 0, 0, 1, 1, 1 };
  m_batch = GPUBackend::get().create_batch(
      { { GPUDataType::Float, position, 2, 8 } }, { 1, 2, 0, 2, 3, 1 });
  m_curves = GPUBackend::get().create_batch(
      { { GPUDataType::Float, m_links.data(), 2, 0 } });

  auto node_tree = Editor::get().get_node_tree();
  for (usize i = 0; i < 5; ++i) {
    node_tree->add_node<NODE_TRANSFORM>(vec2(-100, (f32(i) * 150) - 400));
  }

  m_node_ssbo = GPUBackend::get().create_storage_buffer();
  m_node_ssbo->upload_data(node_tree->get_data(),
                           node_tree->get_size() * sizeof(NodeType));

  m_keymaps.insert(
      { Key::Tab, [&] { m_framebuffer_bit = not m_framebuffer_bit; } });
}

auto NodeEditor::add_node(usize type) -> void {
  auto position = region_space_mouse_position() - vec2(NODE_SIZE);
  auto node_tree = Editor::get().get_node_tree();
  auto& node = node_tree->add_node(type, position);
  m_node_ssbo->upload_data(node_tree->get_data(),
                           node_tree->get_size() * sizeof(NodeType));
  m_active_operator = new OpNodeTransform(this);
}

auto NodeEditor::on_event_imp(Event& event) -> void {
  auto operator_execute = [&] {
    if (auto op = m_active_operator) {
      auto result = op->execute(event);
      if (result != OP_RUNNING) {
        delete m_active_operator;
        m_active_operator = nullptr;
      }
    }
  };

  switch (event.kind) {
    case Event::MouseDown: {
      auto node_tree = Editor::get().get_node_tree();
      auto mouse = relative_mouse_position();
      auto pixel = 0u;
      pixel = mouse.x > m_framebuffer->get_width() or
                      mouse.y > m_framebuffer->get_height()
                  ? 0
                  : m_framebuffer->read_pixel(1, mouse.x, mouse.y);
      if (not pixel & 0xffffff) {
        node_tree->set_active_id(0);
        return;
      }
      auto id = pixel & 0xffffff;
      auto extra = (pixel & 0xff000000) >> 24;
      auto flags = NodeExtraFlags(extra);
      if (flags.visualize) {
        node_tree->set_visualized(id);
        auto* node = node_tree->node_from_id(id);
        node->update(*node);
        node_tree->set_active_id(id);
      } else if (flags.socket_in) {
        auto* node = node_tree->node_from_id(id);
        if (m_active_operator) {
          m_active_operator->set_data_ptr(node);
          m_active_operator->execute(event);
          delete m_active_operator;
          m_active_operator = nullptr;
          update_node([&](auto* node) {});
          return;
        }
      } else if (flags.socket_out) {
        if (m_active_operator) {
          operator_execute();
          return;
        }
        auto* node = node_tree->node_from_id(id);
        m_active_operator = new OpNodeLinkCreateNew(this, node);
        m_active_operator->execute(event);
      } else {
        if (m_active_operator) {
          m_active_operator->execute(event);
          delete m_active_operator;
          m_active_operator = nullptr;
          update_node([&](auto* node) {});
        }
        node_tree->set_active_id(id);
      }
    } break;
    case Event::MouseMove: {
      if (event.button == MouseButton::Left) {
        if (OpNodeTransform(this).execute(event) == OP_CANCELLED) {
          return;
        }
      }
      operator_execute();
      update_node([&](auto* node) {});
    } break;
    case Event::KeyDown: {
      operator_execute();
      update_node([&](auto* node) {});
      if (m_keymaps.contains(event.key)) {
        m_keymaps[event.key]();
      }
    } break;
  }
}

auto NodeEditor::update() -> void {
  m_debug_message = m_active_operator ? "Operator" : "";
  auto node_tree = Editor::get().get_node_tree();
  m_debug_message += fmt::format("\n{}", node_tree->get_links().size());

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
  if (ImGui::BeginPopupContextWindow("Add Menu",
                                     ImGuiPopupFlags_MouseButtonRight)) {
    for (usize i = 1; i < TOTAL_NODES; ++i) {
      if (ImGui::MenuItem(node_names[i])) {
        add_node(i);
      }
    }

    ImGui::EndPopup();
  }
  ImGui::PopStyleVar();

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
    Font::get().render_text_center(node_names[node.type->type], loc.x + 200,
                                   loc.y + 100, 1);
  }
  Font::get().submit();
  GPUState::get().set_blend(Blend::None);
  m_framebuffer->unbind();

  auto& texture = m_framebuffer->get_slot(m_framebuffer_bit).texture;
  ImGui::Image(texture->get_id(), ImVec2(m_size.x, m_size.y),
               ImVec2 { 0.0, 0.0 }, ImVec2 { 1.0, -1.0 });
}

} // namespace zod
