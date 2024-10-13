#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "application/context.hh"
#include "viewport.hh"
#include "widgets/button.hh"

namespace zod {

static auto grid = true;

Viewport::Viewport()
    : Panel("Viewport", unique<PerspectiveCamera>(64, 64, 90.0f, 0.01f, 100.0f),
            false),
      m_width(64), m_height(64) {
  m_framebuffer->bind();
  GPUAttachment attach = { GPUBackend::get().create_texture(
      GPUTextureType::Texture2D, GPUTextureFormat::RGBA8, m_width, m_height,
      false) };
  m_framebuffer->add_color_attachment(attach);
  m_framebuffer->add_depth_attachment();
  m_framebuffer->check();
  m_framebuffer->unbind();

  m_shader = GPUBackend::get().create_shader("view_3d");
  m_shader->init_vertex_shader(g_view3d_vert);
  m_shader->init_fragment_shader(g_view3d_frag);
  m_shader->compile();

  f32 position[] = {
    -1, -1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1,
    -1, -1, 1,  1, -1, 1,  1, 1, 1,  -1, 1, 1,
  };

  auto format = std::vector<GPUBufferLayout> {
    { GPUDataType::Float, position, 3, 24 },
  };
  m_cubemap_batch = GPUBackend::get().create_batch(
      format, { 1, 2, 0, 2, 3, 0, 6, 2, 1, 1, 5, 6, 6, 5, 4, 4, 7, 6,
                6, 3, 2, 7, 3, 6, 3, 7, 0, 7, 4, 0, 5, 1, 0, 4, 5, 0 });

  m_cubemap_shader = GPUBackend::get().create_shader("cubemap");
  m_cubemap_shader->init_vertex_shader(g_cubemap_vert);
  m_cubemap_shader->init_fragment_shader(g_cubemap_frag);
  m_cubemap_shader->compile();

  // Shader taken from
  // https://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
  m_grid_shader = GPUBackend::get().create_shader("grid");
  m_grid_shader->init_vertex_shader(g_grid_vert);
  m_grid_shader->init_fragment_shader(g_grid_frag);
  m_grid_shader->compile();
}

auto Viewport::draw_cubemap() -> void {
  GPUState::get().set_depth_test(Depth::LessEqual);
  m_cubemap_shader->bind();
  m_cubemap_batch->draw(m_cubemap_shader);
  GPUState::get().set_depth_test(Depth::Less);
}

auto Viewport::draw_grid() -> void {
  m_grid_shader->bind();
  m_grid_shader->uniform("u_color", vec3(0.25f));
  GPUState::get().draw_immediate(6);
}

auto Viewport::draw_axes() -> void {}

auto Viewport::on_event_imp(Event& event) -> void {
  switch (event.kind) {
    case Event::MouseMove: {
    } break;
  }
}

auto Viewport::update(Shared<GPUBatch> batch) -> void {
  m_framebuffer->bind();
  m_uniform_buffer->bind();
  m_framebuffer->clear();
  GPUState::get().set_depth_test(Depth::Less);
  GPUState::get().set_blend(Blend::Alpha);
  GPU_TIME("mesh", {
    m_shader->bind();
    batch->draw(m_shader);
  });
  GPU_TIME("cubemap", { draw_cubemap(); });
  if (grid) {
    GPU_TIME("grid", { draw_grid(); });
  }
  GPUState::get().set_depth_test(Depth::None);
  GPUState::get().set_blend(Blend::None);
  m_framebuffer->unbind();

  auto& texture = m_framebuffer->get_slot(0).texture;
  ImGui::Image(texture->get_id(), ImVec2(m_size.x, m_size.y),
               ImVec2 { 0.0, 0.0 }, ImVec2 { 1.0, -1.0 });
  Button("Grid", grid);
}

} // namespace zod
