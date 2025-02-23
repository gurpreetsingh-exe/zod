#include "./shader.hh"

namespace zod {

GLShader::GLShader(GPUShaderCreateInfo info) : GPUShader(info) {
  m_id = glCreateProgram();
  compile(info);
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
    auto info_log = Vector<char>(usize(length));
    glGetShaderInfoLog(shader, length, &length, &info_log[0]);
    glDeleteShader(shader);
    eprintln("{}", info_log.data());
  }

  return shader;
}

auto GLShader::init_vertex_and_fragment_shader(const char* vertex_source,
                                               const char* fragment_source)
    -> void {
  m_vert = create_shader(GL_VERTEX_SHADER, vertex_source);
  m_frag = create_shader(GL_FRAGMENT_SHADER, fragment_source);
}

auto GLShader::init_compute_shader(const char* source) -> void {
  m_comp = create_shader(GL_COMPUTE_SHADER, source);
}

auto GLShader::compile(GPUShaderCreateInfo info) -> void {
  auto bits = info.shader_bits();
  auto link_program_or_crash = [&] {
    glLinkProgram(m_id);
    int is_linked;
    glGetProgramiv(m_id, GL_LINK_STATUS, &is_linked);
    if (is_linked == GL_FALSE) {
      int length;
      glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &length);
      auto info_log = Vector<char>(usize(length));
      glGetProgramInfoLog(m_id, length, &length, &info_log[0]);
      glDeleteProgram(m_id);
      eprintln("{}", info_log.data());
    }
  };

  switch (bits) {
    case GPU_VERTEX_SHADER_BIT | GPU_FRAGMENT_SHADER_BIT: {
      init_vertex_and_fragment_shader(info.get_vertex_source(),
                                      info.get_fragment_source());
      glAttachShader(m_id, m_vert);
      glAttachShader(m_id, m_frag);
      link_program_or_crash();
      glDetachShader(m_id, m_vert);
      glDetachShader(m_id, m_frag);
    } break;
    case GPU_COMPUTE_SHADER_BIT: {
      init_compute_shader(info.get_compute_source());
      glAttachShader(m_id, m_comp);
      link_program_or_crash();
      glDetachShader(m_id, m_comp);
    } break;
    default: {
      eprintln("invalid bits found when compiling shader");
    } break;
  }
}

auto GLShader::bind() -> void { glUseProgram(m_id); }

auto GLShader::unbind() -> void { glUseProgram(0); }

auto GLShader::dispatch(u32 x, u32 y, u32 z) -> void {
  glDispatchCompute(x, y, z);
}

auto GLShader::get_uniform_location(const String& name) -> GLuint {
  if (not m_uniforms.contains(name)) {
    m_uniforms[name] = glGetUniformLocation(m_id, name.c_str());
  }
  return m_uniforms[name];
}

auto GLShader::uniform_uint(const String& name, const u32* n, usize size)
    -> void {
  switch (size) {
    case 1: {
      glUniform1ui(get_uniform_location(name), *n);
    } break;
    default:
      ZASSERT(false, "{}", size);
  }
}

auto GLShader::uniform_int(const String& name, const i32* n, usize size)
    -> void {
  switch (size) {
    case 1: {
      glUniform1i(get_uniform_location(name), *n);
    } break;
    default:
      ZASSERT(false);
  }
}

auto GLShader::uniform_float(const String& name, const f32* v, usize size)
    -> void {
  switch (size) {
    case 2: {
      glUniform2f(get_uniform_location(name), v[0], v[1]);
    } break;
    case 3: {
      glUniform3f(get_uniform_location(name), v[0], v[1], v[2]);
    } break;
    case 16: {
      glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, v);
    } break;
    default:
      ZASSERT(false);
  }
}

} // namespace zod
