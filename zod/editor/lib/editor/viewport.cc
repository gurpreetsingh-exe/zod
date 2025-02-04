#define IMGUI_DEFINE_MATH_OPERATORS
#include <glad/glad.h>
#include <imgui.h>

#include "context.hh"
#include "timer.hh"
#include "viewport.hh"
#include "widgets/button.hh"

namespace zod {

static auto grid = true;
static auto render = false;
static auto cursor_position = 0.0f;

constexpr auto padding = 4.0f;
constexpr auto inner = 4.0f;

static auto Button(const char* name, bool& enabled) -> void {
  ImGui::SetNextItemAllowOverlap();
  auto button_size = ImGui::CalcTextSize(name);
  cursor_position = ImGui::GetWindowContentRegionMax().x - button_size.x -
                    padding - inner * 2 - cursor_position;
  auto cursor =
      ImVec2(cursor_position, ImGui::GetWindowContentRegionMin().y + padding);
  cursor_position = button_size.x + padding + inner * 2;
  ImGui::SetCursorPos(cursor);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(inner, inner));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.f);
  if (enabled) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0.45f, 0.82f, 1));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0.45f, 0.82f, 1));
  }
  ImGui::Button(name);
  if (enabled) {
    ImGui::PopStyleColor(2);
  }
  if (ImGui::IsItemClicked(0)) {
    enabled = not enabled;
  }
  ImGui::PopStyleVar(2);
}

static auto load_env() -> Shared<GPUTexture> {
  const auto& env = ZCtxt::get().get_env();
  const auto path = fs::path(env.hdr.s);
  return fs::exists(path) ? GPUBackend::get().create_texture(
                                GPUTextureType::TextureCube, path)
                          : nullptr;
}

Viewport::Viewport()
    : SPanel("Viewport",
             unique<PerspectiveCamera>(64, 64, 90.0f, 0.01f, 100.0f), false),
      m_width(64), m_height(64) {
  m_framebuffer->bind();
  GPUAttachment attach = { GPUBackend::get().create_texture(
      GPUTextureType::Texture2D, GPUTextureFormat::RGBA8, m_width, m_height,
      false) };
  m_framebuffer->add_color_attachment(attach);
  m_framebuffer->add_depth_attachment();
  m_framebuffer->check();
  m_framebuffer->unbind();

  m_shader =
      GPUBackend::get().create_shader(GPUShaderCreateInfo("view_3d")
                                          .vertex_source(g_view3d_vert_src)
                                          .fragment_source(g_view3d_frag_src));

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

  m_cubemap_shader =
      GPUBackend::get().create_shader(GPUShaderCreateInfo("cubemap")
                                          .vertex_source(g_cubemap_vert_src)
                                          .fragment_source(g_cubemap_frag_src));

  m_cubemap = load_env();

  // Shader taken from
  // https://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
  m_grid_shader =
      GPUBackend::get().create_shader(GPUShaderCreateInfo("grid")
                                          .vertex_source(g_grid_vert_src)
                                          .fragment_source(g_grid_frag_src));
}

auto Viewport::draw_cubemap() -> void {
  if (not m_cubemap) {
    m_framebuffer->clear_color(vec4(1.0f, 0.0f, 1.0f, 1.0f));
    return;
  }
  GPUState::get().set_depth_test(Depth::LessEqual);
  m_cubemap_shader->bind();
  m_cubemap->bind();
  m_cubemap_shader->uniform_int("u_cubemap", ADDR(0));
  m_cubemap_batch->draw(m_cubemap_shader);
  GPUState::get().set_depth_test(Depth::Less);
}

auto Viewport::draw_grid() -> void {
  m_grid_shader->bind();
  m_grid_shader->uniform_float("u_color", value_ptr(vec3(0.25f)), 3);
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
  auto t = ZCtxt::get().get_texture();
  if (m_size != t->get_size()) {
    t->resize(m_size.x, m_size.y);
  }
  if (render) {
    GPU_TIME("compute", {
      auto shader = ZCtxt::get().get_rd_shader();
      shader->bind();
      shader->uniform_uint("u_width", ADDR(u32(m_size.x)));
      shader->uniform_uint("u_height", ADDR(u32(m_size.y)));
      t->bind();
      glBindImageTexture(0, (u32)(intptr_t)t->get_id(), 0, GL_FALSE, 0,
                         GL_READ_WRITE, GL_RGBA8);
      shader->uniform_int("u_texture", ADDR(0));
      shader->dispatch(m_size.x, m_size.y, 1);
    });
  } else {
    m_framebuffer->bind();
    m_uniform_buffer->bind();
    m_framebuffer->clear();
    auto& env = ZCtxt::get().get_env();
    if (env.mode == LightingMode::SolidColor) {
      m_framebuffer->clear_color(vec4(env.color.v3, 1.0f));
    }
    GPUState::get().set_depth_test(Depth::Less);
    GPUState::get().set_blend(Blend::Alpha);
    GPU_TIME("mesh", {
      m_shader->bind();
      batch->draw(m_shader);
    });
    if (env.mode == LightingMode::Texture) {
      if (env.hdr.needs_update) {
        m_cubemap = load_env();
      }
      GPU_TIME("cubemap", { draw_cubemap(); });
    }
    if (grid) {
      GPU_TIME("grid", { draw_grid(); });
    }
    GPUState::get().set_depth_test(Depth::None);
    GPUState::get().set_blend(Blend::None);
    m_framebuffer->unbind();
  }

  auto& texture = render ? t : m_framebuffer->get_slot(0).texture;
  ImGui::Image(texture->get_id(), ImVec2(m_size.x, m_size.y),
               ImVec2 { 0.0, 0.0 }, ImVec2 { 1.0, -1.0 });
  Button("Grid", grid);
  Button("Render", render);
  cursor_position = 0.0f;
}

} // namespace zod
