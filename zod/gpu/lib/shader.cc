#include "shader.hh"

namespace zod {

auto ShaderLibrary::add(const std::string& name, Shared<GPUShader> shader)
    -> void {
  if (m_shaders.contains(name)) {
    fmt::println("shader `{}` already present", name);
  }
  m_shaders.insert({ name, shader });
}

auto ShaderLibrary::get(const std::string& name) -> Shared<GPUShader> {
  ZASSERT(m_shaders.contains(name), "shader `{}` not found", name);
  return m_shaders[name];
}

} // namespace zod
