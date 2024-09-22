#include "application/context.hh"

#include "io/obj.hh"
#include "node_editor.hh"
#include "outliner.hh"
#include "viewport.hh"

namespace zod {

class ZCtxt;
static ZCtxt* g_zcx = nullptr;

auto ZCtxt::get() -> ZCtxt& { return *g_zcx; }

auto ZCtxt::create() -> void {
  ZASSERT(not g_zcx);
  g_zcx = new ZCtxt();
}

auto ZCtxt::drop() -> void { delete g_zcx; }

ZCtxt::ZCtxt()
    : m_window(Window::create(1280, 720, "Zod")),
      m_framebuffer(GPUBackend::get().create_framebuffer(1280, 720)) {
  m_imgui_layer = unique<ImGuiLayer>(m_window->get_handle());
  auto [w, h] = m_window->get_size();
  m_framebuffer->bind();
  GPUAttachment attach = { GPUBackend::get().create_texture(
      GPUTextureType::Texture2D, GPUTextureFormat::RGBA8, w, h, false) };
  m_framebuffer->add_color_attachment(attach);
  m_framebuffer->check();
  m_framebuffer->unbind();
  m_framebuffer->resize(w, h);

#define CREATE_SHADER(n, vert, frag)                                           \
  auto n = GPUBackend::get().create_shader(#n);                                \
  n->init_vertex_shader(vert);                                                 \
  n->init_fragment_shader(frag);                                               \
  n->compile();

  CREATE_SHADER(uv, g_fullscreen, g_uv);
  CREATE_SHADER(quad, g_fullscreen, g_texture);
  CREATE_SHADER(rect, g_rect_vert, g_rect_frag);
  CREATE_SHADER(round_panel, g_vertex_2d, g_round_panel);

  m_window->set_event_callback(std::bind(&ZCtxt::on_event, this, ph::_1));
  m_ssbo = GPUBackend::get().create_storage_buffer();
  m_vertex_buffer = GPUBackend::get().create_storage_buffer();
}

auto ZCtxt::on_event(Event& event) -> void {
  auto pos = m_window->get_mouse_pos();
  switch (event.kind) {
    case Event::MouseDown: {
    } break;
    case Event::MouseMove: {
    } break;
    case Event::MouseUp: {
    } break;
    case Event::WindowResize: {
      // auto w = event.size[0];
      // auto h = event.size[1];
      // f32 b = border * factor;
      // f32 pw = w - b * 2;
      // f32 ph = h - b * 2;
      // m_framebuffer->resize(w, h);
    } break;
    default:
      break;
  }
}

auto ZCtxt::run(fs::path path) -> void {
  auto uuid = m_asset_manager.load(path);
  auto* mesh = m_asset_manager.get(uuid);
  auto vertex_buffer_out = GPUBackend::get().create_storage_buffer();
  m_vertex_buffer->upload_data(mesh->points.data(),
                               mesh->points.size() * sizeof(vec3));
  vertex_buffer_out->upload_data(mesh->points.data(),
                                 mesh->points.size() * sizeof(vec3));

  m_ssbo->upload_data(mesh->normals.data(),
                      mesh->normals.size() * sizeof(vec3));

  auto points = mesh->points.size();
  {
    auto indices = std::vector<u32>();
    for (const auto& prim : mesh->prims) {
      for (auto i : prim.points) { indices.push_back(i); }
    }
    m_batch = GPUBackend::get().create_batch({}, indices);
  }

  auto round_panel = GPUBackend::get().get_shader("round_panel");
  auto quad = GPUBackend::get().get_shader("quad");
  auto rect = GPUBackend::get().get_shader("rect");

  auto transform = GPUBackend::get().create_shader("transform");
  transform->init_compute_shader(g_transform_comp);
  transform->compile();

  constexpr vec4 base = { 30. / 255., 30. / 255., 46. / 255., 1.0f };
  constexpr vec4 mantle = { 0.07f, 0.08f, 0.08f, 1.0f };
  vec3 surface0 = { 0.15f, 0.16f, 0.17f };
  auto viewport = Viewport();
  auto node_editor = NodeEditor();

  m_window->is_running([&] {
    GPU_TIME("main-loop", {
      m_imgui_layer->begin_frame();
      m_imgui_layer->update([&] {
        ImGui::Begin("DebugEditor");
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("Delta Time: %.3f ms", 1000.0f / io.Framerate);
        static bool vsync = true;
        if (ImGui::Checkbox("V-Sync", &vsync)) {
          glfwSwapInterval(vsync);
        }

        static auto offset = vec3(0.0f);

        if (ImGui::DragFloat3("offset", &offset[0])) {
          transform->bind();
          transform->uniform("u_num_vertices", u32(points));
          transform->uniform("u_offset", offset);
          m_vertex_buffer->bind(2);
          vertex_buffer_out->bind(3);
          auto workgroup_size = u32(std::ceil(points / 64.0f));
          transform->dispatch(workgroup_size, 1, 1);
        }

        ImGui::Separator();
        for (const auto& [name, time] : m_times) {
          ImGui::Text("%s: %.3f ms", name.c_str(), time);
        }

        ImGui::End();

        ImGui::Begin("Outliner");
        ImGui::End();

        ImGui::Begin("Properties");
        node_editor.draw_props();
        ImGui::End();

        m_ssbo->bind();
        vertex_buffer_out->bind(2);
        GPU_TIME("viewport", { viewport.update(m_batch); });
        node_editor.update();
      });
      m_imgui_layer->end_frame();
    });
  });

  delete mesh;
}

} // namespace zod
