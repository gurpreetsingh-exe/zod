#include "opengl/shader.hh"

namespace zod {

GLShader::GLShader(std::string name) : GPUShader(std::move(name)) {
  m_id = glCreateProgram();
}

auto GLShader::create_shader(GLuint type, const char* source) -> GLuint {
  ZASSERT(m_id, "shader program not found");
  auto shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, 0);
  glCompileShader(shader);

  int result;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    int length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    auto info_log = std::vector<char>(usize(length));
    glGetShaderInfoLog(shader, length, &length, &info_log[0]);
    glDeleteShader(shader);
    eprintln("{}", info_log.data());
  }

  return shader;
}

auto GLShader::init_vertex_shader(const char* source) -> void {
  m_vert = create_shader(GL_VERTEX_SHADER, source);
}

auto GLShader::init_fragment_shader(const char* source) -> void {
  m_frag = create_shader(GL_FRAGMENT_SHADER, source);
}

auto GLShader::init_compute_shader(const char* source) -> void {
  m_comp = create_shader(GL_COMPUTE_SHADER, source);
}

auto GLShader::compile() -> void {
  if (m_comp) {
    ZASSERT(m_vert == 0 and m_frag == 0);
    glAttachShader(m_id, m_comp);
  } else {
    glAttachShader(m_id, m_vert);
    glAttachShader(m_id, m_frag);
  }
  glLinkProgram(m_id);

  int is_linked;
  glGetProgramiv(m_id, GL_LINK_STATUS, &is_linked);
  if (is_linked == GL_FALSE) {
    int length;
    glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &length);

    auto info_log = std::vector<char>(usize(length));
    glGetProgramInfoLog(m_id, length, &length, &info_log[0]);

    glDeleteProgram(m_id);
    if (m_comp) {
      glDeleteShader(m_comp);
    } else {
      glDeleteShader(m_vert);
      glDeleteShader(m_frag);
    }
    eprintln("{}", info_log.data());
  }

  if (m_comp) {
    glDetachShader(m_id, m_comp);
  } else {
    glDetachShader(m_id, m_vert);
    glDetachShader(m_id, m_frag);
  }
}

auto GLShader::bind() -> void { glUseProgram(m_id); }

auto GLShader::unbind() -> void { glUseProgram(0); }

auto GLShader::dispatch(u32 x, u32 y, u32 z) -> void {
  glDispatchCompute(x, y, z);
}

auto GLShader::get_uniform_location(const std::string& name) -> GLuint {
  if (not m_uniforms.contains(name)) {
    m_uniforms[name] = glGetUniformLocation(m_id, name.c_str());
  }
  return m_uniforms[name];
}

auto GLShader::uniform(const std::string& name, u32 n) -> void {
  glUniform1ui(get_uniform_location(name), n);
}

auto GLShader::uniform(const std::string& name, i32 n) -> void {
  glUniform1i(get_uniform_location(name), n);
}

auto GLShader::uniform(const std::string& name, vec3 vec) -> void {
  glUniform3f(get_uniform_location(name), vec.x, vec.y, vec.z);
}

auto GLShader::uniform(const std::string& name, const mat4& mat) -> void {
  glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, value_ptr(mat));
}

} // namespace zod
