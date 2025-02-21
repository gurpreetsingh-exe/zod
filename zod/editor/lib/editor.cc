#include "editor.hh"

#include "core/components.hh"
#include "core/io/obj.hh"
#include "core/runtime.hh"
#include "environment_light.hh"
#include "gpu/timer.hh"
#include "node_editor.hh"
#include "node_properties.hh"
#include "outliner.hh"
#include "properties.hh"
#include "viewport.hh"
#include "widgets/layout.hh"

namespace zod {

Editor* g_editor;

auto Editor::get() -> Editor& { return *g_editor; }

struct DrawElementsIndirectCommand {
  uint count;
  uint instanceCount;
  uint firstIndex;
  uint baseVertex;
  uint baseInstance;
};

Editor::Editor()
    : ILayer(), m_imgui_layer(unique<ImGuiLayer>(
                    Application::get().active_window().get_native_handle())),
      m_layout(unique<Layout>()), m_node_tree(shared<NodeTree>()),
      m_ssbo(GPUBackend::get().create_storage_buffer()),
      m_vertex_buffer(GPUBackend::get().create_storage_buffer()),
      m_vertex_buffer_out(GPUBackend::get().create_storage_buffer()),
      m_texture(GPUBackend::get().create_texture(
          GPUTextureType::Texture2D, GPUTextureFormat::RGBA8, 32, 32, false)),
      m_rd_shader(GPUBackend::get().create_shader(
          GPUShaderCreateInfo("rd").compute_source(g_rd_comp_src))),
      m_matrix_buffer(GPUBackend::get().create_storage_buffer()) {
  g_editor = this;
}

auto Editor::setup() -> void {
  auto args = Application::get().args();
  if (args.size() < 2) {
    eprintln("no input file");
  }
  auto path = fs::path(args[1]);
  if (not fs::exists(path)) {
    eprintln("\"{}\" does not exist", path.string());
  }
  if (fs::is_directory(path)) {
    eprintln("\"{}\" is a directory", path.string());
  }

  auto uuid = m_asset_manager.load(path);
  auto* mesh = m_asset_manager.get(uuid);
  m_vertex_buffer->upload_data(nullptr, 1024 * 1024);
  m_vertex_buffer_out->upload_data(nullptr, 1024 * 1024);
  m_vertex_buffer->update_data(mesh->points.data(),
                               mesh->points.size() * sizeof(vec3));
  m_vertex_buffer_out->update_data(mesh->points.data(),
                                   mesh->points.size() * sizeof(vec3));
  m_ssbo->upload_data(mesh->normals.data(),
                      mesh->normals.size() * sizeof(vec3));
  m_matrix_buffer->upload_data(ADDROF(*TransformComponent()), sizeof(mat4));

  u32 points = mesh->points.size();
  {
    auto indices = std::vector<u32>();
    for (const auto& prim : mesh->prims) {
      for (auto i : prim.points) { indices.push_back(i); }
    }
    m_batch = GPUBackend::get().create_batch({}, indices);
    DrawElementsIndirectCommand indirect = { .count = u32(indices.size()),
                                             .instanceCount = 1,
                                             .firstIndex = 0,
                                             .baseVertex = 0,
                                             .baseInstance = 0 };
    m_batch->upload_indirect(&indirect, sizeof(indirect));
  }

  m_mesh = mesh;
  constexpr vec4 base = { 30. / 255., 30. / 255., 46. / 255., 1.0f };
  constexpr vec4 mantle = { 0.07f, 0.08f, 0.08f, 1.0f };
  vec3 surface0 = { 0.15f, 0.16f, 0.17f };
  m_layout->add_area(shared<Viewport>());
  m_layout->add_area(shared<NodeEditor>());
  m_layout->add_area(shared<Properties>());
  m_layout->add_area(shared<NodeProperties>());
  m_layout->add_area(shared<EnvironmentLight>());
  m_layout->add_area(shared<Outliner>());
}

auto Editor::on_event(Event& event) -> void {
  if (auto* area = m_layout->active()) {
    area->on_event(event);
  }
}

auto Editor::recompute_batch() -> void {
  auto& scene = Runtime::get().scene();
  auto view = scene->view<TransformComponent>();
  auto* matrix = new f32[view.size() * 16];
  auto i = usize(0);
  auto buffer_size = usize(0);
  for (auto entity_id : view) {
    auto entity = Entity(entity_id, std::addressof(scene));
    auto* mesh = entity.has_component<StaticMeshComponent>()
                     ? entity.get_component<StaticMeshComponent>().mesh
                     : nullptr;
    auto mat = *entity.get_component<TransformComponent>();
    std::memcpy(&matrix[i * 16], ADDROF(mat), sizeof(mat4));
    buffer_size += mesh ? mesh->points.size() * sizeof(vec3) : 0;
    m_matrix_offset_map[entity] = i * sizeof(mat4);
    ++i;
  }
  m_matrix_buffer->upload_data(matrix, sizeof(mat4) * i);
  delete[] matrix;

  auto offset = usize(0);
  auto indices = std::vector<u32>();
  auto indirect = std::vector<DrawElementsIndirectCommand>();
  i = usize(0);
  for (auto entity_id : view) {
    auto entity = Entity(entity_id, std::addressof(scene));
    auto* mesh = entity.has_component<StaticMeshComponent>()
                     ? entity.get_component<StaticMeshComponent>().mesh
                     : nullptr;
    if (not mesh) {
      indirect.emplace_back(0, 1, 0, 0, i);
      ++i;
      continue;
    }
    m_vertex_buffer->update_data(mesh->points.data(),
                                 mesh->points.size() * sizeof(vec3),
                                 offset * sizeof(vec3));
    m_vertex_buffer_out->update_data(mesh->points.data(),
                                     mesh->points.size() * sizeof(vec3),
                                     offset * sizeof(vec3));
    auto begin = indices.size();
    for (const auto& prim : mesh->prims) {
      for (auto n : prim.points) { indices.push_back(n + offset); }
    }
    indirect.emplace_back(indices.size() - begin, 1, begin, 0, i);
    offset += mesh->points.size();
    ++i;
  }
  m_batch = GPUBackend::get().create_batch({}, indices);

  m_batch->upload_indirect(
      indirect.data(), indirect.size() * sizeof(DrawElementsIndirectCommand));
}

auto Editor::update_matrix(Entity entity, const mat4& mat) -> void {
  auto offset = m_matrix_offset_map[entity];
  m_matrix_buffer->update_data(ADDROF(mat), sizeof(mat4), offset);
}

auto Editor::update() -> void {
  GPU_TIME("main-loop", {
    m_imgui_layer->begin_frame();
    m_imgui_layer->update([&] {
      ImGui::Begin("DebugEditor");
      ImGuiIO& io = ImGui::GetIO();
      ImGui::Text("Delta Time: %.3f ms", 1000.0f / io.Framerate);
      static bool vsync = true;
      if (ImGui::Checkbox("V-Sync", &vsync)) {
        Application::get().active_window().set_vsync(vsync);
      }

      ImGui::Separator();
      for (const auto& [name, time] : GPUTimer::get().timings()) {
        ImGui::Text("%s: %.3f ms", name.c_str(), time);
      }

      ImGui::End();

      m_ssbo->bind();
      m_matrix_buffer->bind(1);
      m_vertex_buffer_out->bind(2);
      static auto g = Geometry();
      m_layout->draw(g);
    });
    m_imgui_layer->end_frame();
  });
}

} // namespace zod
