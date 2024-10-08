#pragma once

#include <glad/glad.h>

#include "../shader.hh"

namespace zod {

class GLShader : public GPUShader {
public:
  GLShader(std::string /* name */);

public:
  auto init_vertex_shader(const char* /* source */) -> void override;
  auto init_fragment_shader(const char* /* source */) -> void override;
  auto init_compute_shader(const char* /* source */) -> void override;
  auto compile() -> void override;
  auto bind() -> void override;
  auto unbind() -> void override;
  auto dispatch(u32 /* x */, u32 /* y */, u32 /* z */) -> void override;
  auto uniform(const std::string&, u32) -> void override;
  auto uniform(const std::string&, i32) -> void override;
  auto uniform(const std::string&, vec3) -> void override;
  auto uniform(const std::string&, const mat4&) -> void override;

private:
  auto create_shader(GLuint /* type */, const char* /* source */) -> GLuint;
  auto get_uniform_location(const std::string&) -> GLuint;

private:
  GLuint m_id;
  GLuint m_vert = 0;
  GLuint m_frag = 0;
  GLuint m_comp = 0;
  std::unordered_map<std::string, GLuint> m_uniforms;
};

} // namespace zod
