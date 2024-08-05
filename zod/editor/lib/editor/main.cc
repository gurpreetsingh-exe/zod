#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "backend.hh"
#include "base/math.hh"
#include "window.hh"

#include "widget.hh"
#include "widgets/layout.hh"
#include "widgets/panel.hh"
#include "widgets/split.hh"

#ifndef NDEBUG
#include "imgui_layer.hh"
#endif

namespace zod {

struct GMesh {
  std::vector<vec2> points;
  std::vector<vec2> uvs;
  std::vector<vec2> size;
};

struct UIUbo {
  glm::mat4 view_projection_mat;
  f32 width;
  f32 height;
};

class ZodCtxt {
public:
  ZodCtxt()
      : m_window(Window::create(1280, 720, "Zod")),
        m_renderer(GPUBackend::get().create_renderer()),
        m_shader_library(unique<ShaderLibrary>()), m_layout(unique<Layout>()),
        m_framebuffer(GPUBackend::get().create_framebuffer(1280, 720)) {
#ifndef NDEBUG
    m_imgui_layer = unique<ImGuiLayer>(m_window->get_handle());
#endif
    // m_framebuffer->bind();
    // GPUAttachment attach = { GPUBackend::get().create_texture(
    //     GPUTextureType::Texture2D, 1280, 720) };
    // m_framebuffer->add_color_attachment(attach);
    // m_framebuffer->check();

    auto quad = GPUBackend::get().create_shader("quad");
    quad->init_vertex_shader(g_fullscreen);
    quad->init_fragment_shader(g_uv);
    quad->compile();
    m_shader_library->add(quad->name, quad);

    auto rect = GPUBackend::get().create_shader("rect");
    rect->init_vertex_shader(g_rect_vert);
    rect->init_fragment_shader(g_rect_frag);
    rect->compile();
    m_shader_library->add(rect->name, rect);

    auto round_panel = GPUBackend::get().create_shader("round_panel");
    round_panel->init_vertex_shader(g_vertex_2d);
    round_panel->init_fragment_shader(g_round_panel);
    round_panel->compile();
    m_shader_library->add(round_panel->name, round_panel);
    m_window->set_event_callback(std::bind(&ZodCtxt::on_event, this, ph::_1));

    // Unique<Split> s1 = unique<Split>(SplitKind::Vertical);
    // s1->add_node(unique<Panel>(), 0.25);
    // s1->add_node(unique<Panel>(), 0.5);
    // s1->add_node(unique<Panel>(), 0.25);
    // m_layout->add_area(std::move(s1));


    Unique<Split> s1 = unique<Split>(SplitKind::Vertical);
    s1->add_node(unique<Panel>(), 0.25);

    Unique<Split> s2 = unique<Split>(SplitKind::Horizontal);
    s2->add_node(unique<Panel>(), 0.25);
    s2->add_node(unique<Panel>(), 0.75);
    s1->add_node(std::move(s2), 0.5);

    Unique<Split> s3 = unique<Split>(SplitKind::Horizontal);
    s3->add_node(unique<Panel>(), 0.6);
    s3->add_node(unique<Panel>(), 0.4);
    s1->add_node(std::move(s3), 0.25);

    m_layout->add_area(std::move(s1));
  }

  auto on_event(Event& event) -> void {
    switch (event.kind) {
      case Event::MouseDown: {
      } break;
      case Event::MouseMove: {
      } break;
      case Event::MouseUp: {
      } break;
      default:
        break;
    }
  }

  auto run() -> void {
    int border = 8;
    auto ubo = GPUBackend::get().create_uniform_buffer(sizeof(UIUbo));
    // std::vector<f32> position = { -1, 1, 1, 1, -1, -1, 1, -1 };
    std::vector<f32> position = { 0, 1, 1, 1, 0, 0, 1, 0 };
    std::vector<f32> uv = { 0, 1, 1, 1, 0, 0, 1, 0 };
    // f32 n = 0.5;
    // std::vector<f32> offsets = { 0, 0, n, 0, n, n, 0, n };
    auto offsets = std::vector<vec2>();
    m_layout->calculate(0, 0, 1, 1);
    m_layout->generate(offsets);
    auto format = std::vector<GPUBufferLayout> {
      { GPUDataType::Float, position.data(), 2, position.size() }, // position
      { GPUDataType::Float, uv.data(), 2, uv.size() },             // uv
      { GPUDataType::Float, offsets.data(), 2, offsets.size() * 2,
        true }, // offsets
    };

    auto corners = GPUBackend::get().create_batch(format, { 0, 1, 2, 2, 1, 3 });

    auto indices = std::vector<u32>();
    for (usize i = 0; i < offsets.size(); i += 4) {
      indices.push_back(i);
      indices.push_back(i + 1);
      indices.push_back(i + 2);
      indices.push_back(i + 2);
      indices.push_back(i + 1);
      indices.push_back(i + 3);
    }

    auto rects = GPUBackend::get().create_batch(
        { { GPUDataType::Float, offsets.data(), 2, offsets.size() * 2 } },
        indices);

    auto panel_fb = GPUBackend::get().create_framebuffer(64, 64);
    panel_fb->bind();
    GPUAttachment attach = { GPUBackend::get().create_texture(
        GPUTextureType::Texture2D, 64, 64, false) };
    panel_fb->add_color_attachment(attach);
    panel_fb->check();

    auto round_panel = m_shader_library->get("round_panel");
    auto quad = m_shader_library->get("quad");
    auto rect = m_shader_library->get("rect");

    f32 old_x = 0;
    f32 old_y = 0;

    m_window->is_running([&] {
#ifndef NDEBUG
      m_imgui_layer->begin_frame();
      m_imgui_layer->update([&] {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DebugEditor");
        ImGui::DragInt("border", &border);
        static f32 factor = 0.4;
        ImGui::DragFloat("Border Factor", &factor, 0.1);
        ImGui::End();

        ImGui::Begin("Panel");
        auto dim = ImGui::GetContentRegionAvail();
        if (dim.x != old_x or dim.y != old_y) {
          f32 b = border * factor;
          auto uiubo =
              UIUbo { glm::ortho(0.f, dim.x, 0.f, dim.y, -1.0f, 1.0f), dim.x - b * 2,
                      dim.y - b * 2 };
          ubo->upload_data(&uiubo, sizeof(UIUbo));
          panel_fb->resize(dim.x, dim.y);
          offsets.clear();
          m_layout->calculate(b, b, dim.x - b * 2, dim.y - b * 2);
          // m_layout->calculate(0, 0, dim.x, dim.y);
          m_layout->generate(offsets);
          corners->update_binding(2, offsets.data(),
                                  offsets.size() * sizeof(vec2));
          rects->update_binding(0, offsets.data(),
                                offsets.size() * sizeof(vec2));
          old_x = dim.x;
          old_y = dim.y;
        }
        auto texture = panel_fb->get_slot(0).texture;
        auto size = texture->get_size();
        ImGui::Image(texture->get_id(), *(ImVec2*)&size, ImVec2 { 0.0, 0.0 },
                     ImVec2 { 1.0, -1.0 });
        ImGui::End();
        ImGui::PopStyleVar();
      });
#endif
      constexpr vec4 base = { 30. / 255., 30. / 255., 46. / 255., 1.0f };
      constexpr vec4 mantle = { 24. / 255., 24. / 255., 37. / 255., 1.0f };
      constexpr glm::vec3 surface0 = { 49. / 255, 50. / 255, 68. / 255 };
      m_renderer->clear_color(mantle);
      ubo->bind();
      panel_fb->bind();
      m_renderer->clear_color(mantle);
      rect->bind();
      rect->uniform("u_border", border);
      rect->uniform("u_color", surface0);
      rects->draw(rect);
      round_panel->bind();
      round_panel->uniform("u_border", border);
      corners->draw_instanced(round_panel, offsets.size());
      panel_fb->unbind();
#ifndef NDEBUG
      m_imgui_layer->end_frame();
#endif
    });
  }

private:
  Unique<Window> m_window;
  Shared<GPURenderer> m_renderer;
  Unique<ShaderLibrary> m_shader_library;
  Unique<Layout> m_layout;
  Shared<GPUFrameBuffer> m_framebuffer;

#if !defined(NDEBUG)
  Unique<ImGuiLayer> m_imgui_layer;
#endif
};

} // namespace zod

auto main() -> int {
  auto zcx = zod::unique<zod::ZodCtxt>();
  zcx->run();
}
