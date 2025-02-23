#pragma once

#include <glad/glad.h>

#include "gpu/shader.hh"

namespace zod {

class GLShader : public GPUShader {
public:
  GLShader(GPUShaderCreateInfo);

public:
  auto bind() -> void override;
  auto unbind() -> void override;
  auto dispatch(u32 /* x */, u32 /* y */, u32 /* z */) -> void override;
  auto uniform_float(const String&, const f32*, usize /* size */)
      -> void override;
  auto uniform_uint(const String&, const u32*, usize /* size */)
      -> void override;
  auto uniform_int(const String&, const i32*, usize /* size */)
      -> void override;

private:
  auto init_vertex_and_fragment_shader(const char* /* vertex_source */,
                                       const char* /* fragment_source */)
      -> void override;
  auto init_compute_shader(const char* /* source */) -> void override;
  auto compile(GPUShaderCreateInfo) -> void override;

  auto create_shader(GLuint /* type */, const char* /* source */) -> GLuint;
  auto get_uniform_location(const String&) -> GLuint;

private:
  GLuint m_id;
  GLuint m_vert = 0;
  GLuint m_frag = 0;
  GLuint m_comp = 0;
  std::unordered_map<String, GLuint> m_uniforms;
};

} // namespace zod
