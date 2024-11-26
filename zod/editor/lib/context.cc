#include "context.hh"

#include "io/obj.hh"
#include "node_editor.hh"
#include "outliner.hh"
#include "properties.hh"
#include "timer.hh"
#include "viewport.hh"
#include "widgets/layout.hh"

namespace zod {

class ZCtxt;
static ZCtxt* g_zcx = nullptr;

auto ZCtxt::get() -> ZCtxt& { return *g_zcx; }

auto ZCtxt::create() -> void {
  ZASSERT(not g_zcx);
  g_zcx = new ZCtxt();
  SApplication::create(g_zcx);
}

auto ZCtxt::drop() -> void { delete g_zcx; }

ZCtxt::ZCtxt() : m_node_tree(shared<NodeTree>()) {
  ZASSERT(not g_zcx);
  init_window("Zod");
  init_font("../third-party/imgui/misc/fonts/DroidSans.ttf");
  m_layout = unique<Layout>();
  m_ssbo = GPUBackend::get().create_storage_buffer();
  m_vertex_buffer = GPUBackend::get().create_storage_buffer();
}

auto ZCtxt::on_event(Event& event) -> void {
  if (auto* area = m_layout->active()) {
    area->on_event(event);
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

  u32 points = mesh->points.size();
  {
    auto indices = std::vector<u32>();
    for (const auto& prim : mesh->prims) {
      for (auto i : prim.points) { indices.push_back(i); }
    }
    m_batch = GPUBackend::get().create_batch({}, indices);
  }

  auto transform = GPUBackend::get().create_shader(
      GPUShaderCreateInfo("transform").compute_source(g_transform_comp));

  constexpr vec4 base = { 30. / 255., 30. / 255., 46. / 255., 1.0f };
  constexpr vec4 mantle = { 0.07f, 0.08f, 0.08f, 1.0f };
  vec3 surface0 = { 0.15f, 0.16f, 0.17f };
  m_layout->add_area(shared<Viewport>());
  m_layout->add_area(shared<NodeEditor>());
  m_layout->add_area(shared<Properties>());
  m_layout->add_area(shared<Outliner>());
  auto g = Geometry();

  m_window->is_running([&] {
    GPU_TIME("main-loop", {
      m_imgui_layer->begin_frame();
      m_imgui_layer->update([&] {
        ImGui::Begin("DebugEditor");
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("Delta Time: %.3f ms", 1000.0f / io.Framerate);
        static bool vsync = true;
        if (ImGui::Checkbox("V-Sync", &vsync)) {
          m_window->set_vsync(vsync);
        }

        static auto offset = vec3(0.0f);

        if (ImGui::DragFloat3("offset", &offset[0])) {
          transform->bind();
          transform->uniform_uint("u_num_vertices", &points);
          transform->uniform_float("u_offset", ADDROF(offset), 3);
          m_vertex_buffer->bind(2);
          vertex_buffer_out->bind(3);
          auto workgroup_size = u32(std::ceil(points / 64.0f));
          transform->dispatch(workgroup_size, 1, 1);
        }

        ImGui::Separator();
        for (const auto& [name, time] : GPUTimer::get().timings()) {
          ImGui::Text("%s: %.3f ms", name.c_str(), time);
        }

        ImGui::End();

        m_ssbo->bind();
        vertex_buffer_out->bind(2);
        m_layout->draw(g);
      });
      m_imgui_layer->end_frame();
    });
  });

  delete mesh;
}

} // namespace zod
