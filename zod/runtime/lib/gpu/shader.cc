#include "gpu/shader.hh"

namespace zod {

auto GPUShaderCreateInfo::vertex_in(VertexIn in) -> GPUShaderCreateInfo& {
  inputs.push_back(in);
  return *this;
}

auto GPUShaderCreateInfo::fragment_out(FragmentOut out)
    -> GPUShaderCreateInfo& {
  outputs.push_back(out);
  return *this;
}

auto GPUShaderCreateInfo::varying(Varying varying) -> GPUShaderCreateInfo& {
  varyings.push_back(varying);
  return *this;
}

auto GPUShaderCreateInfo::vertex_source(const char* src)
    -> GPUShaderCreateInfo& {
  m_shader_bits |= GPU_VERTEX_SHADER_BIT;
  m_vertex_source = src;
  return *this;
}

auto GPUShaderCreateInfo::fragment_source(const char* src)
    -> GPUShaderCreateInfo& {
  m_shader_bits |= GPU_FRAGMENT_SHADER_BIT;
  m_fragment_source = src;
  return *this;
}

auto GPUShaderCreateInfo::compute_source(const char* src)
    -> GPUShaderCreateInfo& {
  m_shader_bits |= GPU_COMPUTE_SHADER_BIT;
  m_compute_source = src;
  return *this;
}

auto ShaderLibrary::add(const String& name, SharedPtr<GPUShader> shader)
    -> void {
  if (m_shaders.contains(name)) {
    fmt::println("shader `{}` already present", name);
  }
  m_shaders.insert({ name, shader });
}

auto ShaderLibrary::get(const String& name) -> SharedPtr<GPUShader> {
  ZASSERT(m_shaders.contains(name), "shader `{}` not found", name);
  return m_shaders[name];
}

} // namespace zod
