#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "backend.hh"
#include "base/math.hh"
#include "window.hh"
#include "viewport.hh"

#include "widget.hh"
#include "widgets/layout.hh"
#include "widgets/panel.hh"
#include "widgets/split.hh"

#include "imgui_layer.hh"

namespace zod {

struct UIUbo {
  glm::mat4 view_projection_mat;
  f32 width;
  f32 height;
};

int border = 8;
f32 factor = 0.2;

class ZodCtxt {
public:
  ZodCtxt()
      : m_window(Window::create(1280, 720, "Zod")),
        m_renderer(GPUBackend::get().create_renderer()),
        m_shader_library(unique<ShaderLibrary>()), m_layout(unique<Layout>()),
        m_framebuffer(GPUBackend::get().create_framebuffer(1280, 720)),
        m_uniform_buffer(
            GPUBackend::get().create_uniform_buffer(sizeof(UIUbo))) {
    m_imgui_layer = unique<ImGuiLayer>(m_window->get_handle());
    auto [w, h] = m_window->get_size();
    m_framebuffer->bind();
    GPUAttachment attach = { GPUBackend::get().create_texture(
        GPUTextureType::Texture2D, w, h, false) };
    m_framebuffer->add_color_attachment(attach);
    m_framebuffer->check();
    m_framebuffer->unbind();
    m_framebuffer->resize(w, h);

#define CREATE_SHADER(n, vert, frag)                                           \
  auto n = GPUBackend::get().create_shader(#n);                                \
  n->init_vertex_shader(vert);                                                 \
  n->init_fragment_shader(frag);                                               \
  n->compile();                                                                \
  m_shader_library->add(n->name, n)

    CREATE_SHADER(quad, g_fullscreen, g_texture);
    CREATE_SHADER(rect, g_rect_vert, g_rect_frag);
    CREATE_SHADER(round_panel, g_vertex_2d, g_round_panel);

    m_window->set_event_callback(std::bind(&ZodCtxt::on_event, this, ph::_1));

    Unique<Split> s1 = unique<Split>(SplitKind::Vertical);
    s1->add_node(unique<Panel>(), 0.25);

    Unique<Split> s2 = unique<Split>(SplitKind::Horizontal);
    s2->add_node(unique<Panel>(), 0.25);
    s2->add_node(unique<Viewport>(), 0.75);
    s1->add_node(std::move(s2), 0.5);

    Unique<Split> s3 = unique<Split>(SplitKind::Horizontal);
    s3->add_node(unique<Panel>(), 0.6);
    s3->add_node(unique<Panel>(), 0.4);
    s1->add_node(std::move(s3), 0.25);

    m_layout->add_area(std::move(s1));
    m_layout->calculate(0, 0, w, h);
    m_layout->init();
  }

  auto on_event(Event& event) -> void {
    switch (event.kind) {
      case Event::MouseDown: {
      } break;
      case Event::MouseMove: {
        auto x = event.mouse[0];
        auto y = event.mouse[1];
        if (auto widget = m_layout->get_widget(x, y)) {
          widget->on_event(event);
          m_current_panel = widget->id;
        }
      } break;
      case Event::MouseUp: {
      } break;
      case Event::WindowResize: {
        auto w = event.size[0];
        auto h = event.size[1];
        f32 b = border * factor;
        f32 pw = w - b * 2;
        f32 ph = h - b * 2;
        m_framebuffer->resize(w, h);
        auto uiubo = UIUbo { glm::ortho(0.f, w, 0.f, h, -1.f, 1.f), pw, ph };
        m_uniform_buffer->upload_data(&uiubo, sizeof(UIUbo));
        m_layout->update(b, b, pw, ph);
      } break;
      default:
        break;
    }
  }

  auto run() -> void {
    auto [w, h] = m_window->get_size();
    f32 b = border * factor;
    auto uiubo = UIUbo { glm::ortho(0.f, f32(w), 0.f, f32(h), -1.0f, 1.0f),
                         w - b * 2, h - b * 2 };
    m_uniform_buffer->upload_data(&uiubo, sizeof(UIUbo));
    auto round_panel = m_shader_library->get("round_panel");
    auto quad = m_shader_library->get("quad");
    auto rect = m_shader_library->get("rect");

    constexpr vec4 base = { 30. / 255., 30. / 255., 46. / 255., 1.0f };
    constexpr vec4 mantle = { 24. / 255., 24. / 255., 37. / 255., 1.0f };
    glm::vec3 surface0 = { 49. / 255, 50. / 255, 68. / 255 };

    m_uniform_buffer->bind();
    m_window->is_running([&] {
      m_imgui_layer->begin_frame();
      m_imgui_layer->update([&] {
        // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DebugEditor");
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("Delta Time: %.3f ms", 1000.0f / io.Framerate);
        static bool vsync = true;
        if (ImGui::Checkbox("V-Sync", &vsync)) {
          glfwSwapInterval(vsync);
        }
        ImGui::Text("PanelID: %zu", m_current_panel);
        ImGui::DragInt("border", &border);
        ImGui::DragFloat("Border Factor", &factor, 0.1);
        ImGui::ColorEdit4("Color", &surface0[0]);
        ImGui::End();
        // ImGui::PopStyleVar();
      });
      m_framebuffer->bind();
      m_renderer->clear_color(mantle);
      m_layout->draw(rect, round_panel, border, surface0);
      m_framebuffer->unbind();

      quad->bind();
      m_framebuffer->get_slot(0).texture->bind();
      quad->uniform("u_texture", 0);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      m_imgui_layer->end_frame();
    });
  }

private:
  Unique<Window> m_window;
  Shared<GPURenderer> m_renderer;
  Unique<ShaderLibrary> m_shader_library;
  Unique<Layout> m_layout;
  Shared<GPUFrameBuffer> m_framebuffer;
  Shared<GPUUniformBuffer> m_uniform_buffer;
  Unique<ImGuiLayer> m_imgui_layer;
  usize m_current_panel;
};

} // namespace zod

auto main() -> int {
  auto zcx = zod::unique<zod::ZodCtxt>();
  zcx->run();
}
