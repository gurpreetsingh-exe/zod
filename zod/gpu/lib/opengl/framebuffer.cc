#include "opengl/framebuffer.hh"
#include "opengl/context.hh"

namespace zod {

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
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);
  glViewport(0, 0, m_width, m_height);
}

auto GLFrameBuffer::unbind() -> void { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

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
  unbind();
}

auto GLFrameBuffer::check() -> void {
  bind();
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

} // namespace zod
