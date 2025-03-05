#include "engine/renderer.hh"
#include "./mesh_batch.hh"
#include "engine/components.hh"
#include "engine/runtime.hh"
#include "gpu/timer.hh"

namespace zod {

static constexpr f32 DEFAULT_FB_SIZE = 512.0f;

GPUShaderCreateInfo view_3d = GPUShaderCreateInfo("view_3d")
                                  .vertex_source(g_view3d_vert_src)
                                  .fragment_source(g_view3d_frag_src);

GPUShaderCreateInfo cubemap = GPUShaderCreateInfo("cubemap")
                                  .vertex_source(g_cubemap_vert_src)
                                  .fragment_source(g_cubemap_frag_src);

Renderer::Renderer()
    : m_framebuffer(GPUBackend::get().create_framebuffer(DEFAULT_FB_SIZE,
                                                         DEFAULT_FB_SIZE)) {
  m_framebuffer->bind();
  GPUAttachment attach = { GPUBackend::get().create_texture(
      GPUTextureType::Texture2D, GPUTextureFormat::RGBA8, DEFAULT_FB_SIZE,
      DEFAULT_FB_SIZE, false) };
  m_framebuffer->add_color_attachment(attach);
  m_framebuffer->add_depth_attachment();
  m_framebuffer->check();
  m_framebuffer->unbind();

  GPUBackend::get().create_shader(view_3d);
  GPUBackend::get().create_shader(cubemap);
}

auto Renderer::resize(f32 w, f32 h) -> void {
  m_framebuffer->resize(w, h);
  auto& scene = Runtime::get().scene();
  if (scene.m_camera == entt::null) {
    return;
  }
  auto& component =
      Entity(scene.m_camera, &scene).get_component<CameraComponent>();
  auto camera = component.camera;
  camera->resize(w, h);
  camera->update_matrix();
}

auto Renderer::tick() -> void {
  auto& scene = Runtime::get().scene();
  auto batch = scene.m_mesh_batch;
  batch->bind();
  scene.m_uniform_buffer->bind();

  m_framebuffer->bind();
  m_framebuffer->clear();
  m_framebuffer->clear_color(vec4(0.1f, 0.1f, 0.1f, 1.0f));
  GPUState::get().set_depth_test(Depth::Less);
  GPUState::get().set_blend(Blend::Alpha);
  GPU_TIME("mesh", {
    auto shader = GPUBackend::get().get_shader("view_3d");
    shader->bind();
    batch->batch().draw_indirect(shader);
  });

  if (scene.m_env != entt::null) {
    auto& env =
        Entity(scene.m_env, &scene).get_component<SkyboxComponent>().env;
    if (env.mode == LightingMode::Texture) {
      GPU_TIME("cubemap", {
        if (not scene.m_cubemap) {
          m_framebuffer->clear_color(vec4(1.0f, 0.0f, 1.0f, 1.0f));
          return;
        }
        GPUState::get().set_depth_test(Depth::LessEqual);
        auto shader = GPUBackend::get().get_shader("cubemap");
        shader->bind();
        scene.m_cubemap->bind();
        shader->uniform_int("u_cubemap", ADDR(0));
        batch->batch<false>().draw(shader);
      });
    } else {
      m_framebuffer->clear_color(vec4(env.color.v3, 1.0f));
    }
  }
  GPUState::get().set_depth_test(Depth::None);
  GPUState::get().set_blend(Blend::None);
  m_framebuffer->unbind();
  scene.m_uniform_buffer->unbind();
}

} // namespace zod
