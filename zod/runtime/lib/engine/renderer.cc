#include "engine/renderer.hh"
#include "./mesh_batch.hh"
#include "engine/components.hh"
#include "engine/runtime.hh"
#include "gpu/timer.hh"

#include "gpu/shader_builtins.hh"

namespace zod {

static constexpr f32 DEFAULT_FB_SIZE = 1024.0f;

GPUShaderCreateInfo view_3d = GPUShaderCreateInfo("view_3d")
                                  .vertex_source(g_view3d_vert_src)
                                  .fragment_source(g_view3d_frag_src);

GPUShaderCreateInfo gbuffer = GPUShaderCreateInfo("gbuffer")
                                  .vertex_source(g_gbuffer_vert_src)
                                  .fragment_source(g_gbuffer_frag_src);

GPUShaderCreateInfo cubemap = GPUShaderCreateInfo("cubemap")
                                  .vertex_source(g_cubemap_vert_src)
                                  .fragment_source(g_cubemap_frag_src);

GPUShaderCreateInfo pbr = GPUShaderCreateInfo("pbr")
                              .vertex_source(g_fullscreen_src)
                              .fragment_source(g_pbr_frag_src);

Renderer::Renderer()
    : m_gbuffer(GPUBackend::get().create_framebuffer(DEFAULT_FB_SIZE,
                                                     DEFAULT_FB_SIZE)),
      m_framebuffer(GPUBackend::get().create_framebuffer(DEFAULT_FB_SIZE,
                                                         DEFAULT_FB_SIZE)) {

  m_gbuffer->bind();
  m_gbuffer->add_color_attachment(GPUBackend::get().create_texture({
      .width = i32(DEFAULT_FB_SIZE),
      .height = i32(DEFAULT_FB_SIZE),
      .format = GPUTextureFormat::RGB8,
  }));
  m_gbuffer->add_color_attachment(GPUBackend::get().create_texture({
      .width = i32(DEFAULT_FB_SIZE),
      .height = i32(DEFAULT_FB_SIZE),
      .format = GPUTextureFormat::RGB32F,
      .data = GPUTextureData::Float,
  }));
  m_gbuffer->add_depth_attachment();
  m_gbuffer->check();
  m_gbuffer->unbind();

  m_framebuffer->bind();
  m_framebuffer->add_color_attachment(GPUBackend::get().create_texture(
      { .width = i32(DEFAULT_FB_SIZE), .height = i32(DEFAULT_FB_SIZE) }));
  m_framebuffer->add_depth_attachment();
  m_framebuffer->check();
  m_framebuffer->unbind();

  GPUBackend::get().create_shader(view_3d);
  GPUBackend::get().create_shader(gbuffer);
  GPUBackend::get().create_shader(cubemap);
  GPUBackend::get().create_shader(pbr);
}

auto Renderer::resize(f32 w, f32 h) -> void {
  m_gbuffer->resize(w, h);
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
  scene.m_camera_buffer->bind(6);

  m_gbuffer->bind();
  m_gbuffer->clear();
  m_gbuffer->clear_color(vec4(0.1f, 0.1f, 0.1f, 1.0f));
  GPUState::get().set_depth_test(Depth::Less);
  GPUState::get().set_blend(Blend::Alpha);
  GPU_TIME("GBuffer", {
    auto shader = GPUBackend::get().get_shader("gbuffer");
    shader->bind();
    shader->uniform_int("u_mega_texture", ADDR(0));
    batch->batch().draw_indirect(shader);
  });
  m_gbuffer->unbind();

  m_framebuffer->bind();
  m_framebuffer->clear();
  m_framebuffer->clear_color(vec4(0.1f, 0.1f, 0.1f, 1.0f));
  GPUState::get().set_depth_test(Depth::None);
  GPUState::get().set_blend(Blend::None);
  GPU_TIME("PBR", {
    auto shader = GPUBackend::get().get_shader("pbr");
    shader->bind();
    m_gbuffer->get_slot(0)->bind();
    shader->uniform_int("u_albedo", ADDR(0));
    m_gbuffer->get_slot(1)->bind(1);
    shader->uniform_int("u_normal", ADDR(1));
    m_gbuffer->bind_depth(2);
    shader->uniform_int("u_depth", ADDR(2));
    GPUState::get().draw_immediate(3);
  });

  // if (scene.m_env != entt::null) {
  //   auto& env =
  //       Entity(scene.m_env, &scene).get_component<SkyboxComponent>().env;
  //   if (env.mode == LightingMode::Texture) {
  //     GPU_TIME("cubemap", {
  //       if (not scene.m_cubemap) {
  //         m_framebuffer->clear_color(vec4(1.0f, 0.0f, 1.0f, 1.0f));
  //         return;
  //       }
  //       GPUState::get().set_depth_test(Depth::LessEqual);
  //       auto shader = GPUBackend::get().get_shader("cubemap");
  //       shader->bind();
  //       scene.m_cubemap->bind();
  //       shader->uniform_int("u_cubemap", ADDR(0));
  //       batch->batch<false>().draw(shader);
  //     });
  //   } else {
  //     m_framebuffer->clear_color(vec4(env.color.v3, 1.0f));
  //   }
  // }
  GPUState::get().set_depth_test(Depth::None);
  GPUState::get().set_blend(Blend::None);
  m_framebuffer->unbind();
  scene.m_camera_buffer->unbind();
}

} // namespace zod
