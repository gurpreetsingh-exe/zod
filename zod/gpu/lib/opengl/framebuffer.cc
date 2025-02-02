#include "opengl/framebuffer.hh"
#include "opengl/context.hh"

namespace zod {

static GLenum g_attachments[4] = {
  GL_COLOR_ATTACHMENT0,
  GL_COLOR_ATTACHMENT1,
  GL_COLOR_ATTACHMENT2,
  GL_COLOR_ATTACHMENT3,
};

GLFrameBuffer::GLFrameBuffer(i32 width, i32 height)
    : GPUFrameBuffer(width, height) {
  // m_width = w;
  // m_height = h;
  // m_viewport[0] = m_scissor[0] = 0;
  // m_viewport[1] = m_scissor[1] = 0;
  // m_viewport[2] = m_scissor[2] = w;
  // m_viewport[3] = m_scissor[3] = h;
  glCreateFramebuffers(1, &m_id);
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

GLFrameBuffer::~GLFrameBuffer() { glDeleteFramebuffers(1, &m_id); }

auto GLFrameBuffer::bind() -> void {
  glGetIntegerv(GL_VIEWPORT, m_view);
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);
  glViewport(0, 0, m_width, m_height);
}

auto GLFrameBuffer::unbind() -> void {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(m_view[0], m_view[1], m_view[2], m_view[3]);
}

auto GLFrameBuffer::resize(i32 width, i32 height) -> void {
  m_width = width;
  m_height = height;
  bind();
  for (auto [i, attach] : rng::enumerate_view(m_color_attachments)) {
    attach.texture->resize(width, height);
    auto gl_tex = std::static_pointer_cast<GLTexture>(attach.texture);
    gl_tex->bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                           gl_tex->m_target, gl_tex->m_id, 0);
  }
  if (m_depth_attachment) {
    glDeleteTextures(1, &m_depth_attachment);
    add_depth_attachment();
  }
  unbind();
}

auto GLFrameBuffer::check() -> void {
  bind();

  if (m_color_attachments.size() > 1) {
    glDrawBuffers(m_color_attachments.size(), g_attachments);
  }

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

#define FORMAT_STATUS(X)                                                       \
  case X: {                                                                    \
    err = #X;                                                                  \
    break;                                                                     \
  }

  const char* err;
  switch (status) {
    FORMAT_STATUS(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
    FORMAT_STATUS(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
    FORMAT_STATUS(GL_FRAMEBUFFER_UNSUPPORTED);
    FORMAT_STATUS(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
    FORMAT_STATUS(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
    FORMAT_STATUS(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
    FORMAT_STATUS(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS);
    FORMAT_STATUS(GL_FRAMEBUFFER_UNDEFINED);
    case GL_FRAMEBUFFER_COMPLETE:
      return;
    default:
      err = "unknown";
      break;
  }
#undef FORMAT_STATUS

  fmt::println(stderr, "GPUFrameBuffer: status {}", err);
  unbind();
}

auto GLFrameBuffer::clear() -> void {
  bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

auto GLFrameBuffer::clear_color(vec4 color) -> void {
  glClearColor(color.x, color.y, color.z, color.w);
}

auto GLFrameBuffer::add_color_attachment(GPUAttachment& attach) -> void {
  auto multisampled = m_samples > 1;
  auto gl_tex = static_pointer_cast<GLTexture>(attach.texture);
  auto target = multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
  ZASSERT(gl_tex->m_target == target);
  gl_tex->bind();
  // if (multisampled) {
  //   glTexImage2DMultisample(target, m_samples, GL_RGBA8, m_width, m_height,
  //                           GL_FALSE);
  // } else {
  //   glTexImage2D(target, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA,
  //                GL_UNSIGNED_BYTE, nullptr);
  // }
  glFramebufferTexture2D(GL_FRAMEBUFFER,
                         GL_COLOR_ATTACHMENT0 + m_color_attachments.size(),
                         target, gl_tex->m_id, 0);
  m_color_attachments.push_back(attach);
}

auto GLFrameBuffer::add_depth_attachment() -> void {
  auto multisampled = m_samples > 1;
  auto target = multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

  glCreateTextures(target, 1, &m_depth_attachment);
  glBindTexture(target, m_depth_attachment);
  glTexStorage2D(target, 1, GL_DEPTH24_STENCIL8, m_width, m_height);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, target,
                         m_depth_attachment, 0);
  glBindTexture(target, 0);
}

} // namespace zod
