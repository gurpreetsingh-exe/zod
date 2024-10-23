#pragma once

#define X(name) extern const char* g_##name;
#include "shaders.def"
#undef X

namespace zod {

class GPUShader {
public:
  std::string name;

public:
  GPUShader(std::string name_) : name(std::move(name_)) {}
  virtual ~GPUShader() = default;
  virtual auto init_vertex_shader(const char* /* source */) -> void = 0;
  virtual auto init_fragment_shader(const char* /* source */) -> void = 0;
  virtual auto init_compute_shader(const char* /* source */) -> void = 0;
  virtual auto compile() -> void = 0;
  virtual auto bind() -> void = 0;
  virtual auto unbind() -> void = 0;
  virtual auto dispatch(u32 /* x */, u32 /* y */, u32 /* z */) -> void = 0;
  virtual auto uniform_float(const std::string&, const f32*,
                             usize /* size */ = 1) -> void = 0;
  virtual auto uniform_uint(const std::string&, const u32*,
                            usize /* size */ = 1) -> void = 0;
  virtual auto uniform_int(const std::string&, const i32*, usize /* size */ = 1)
      -> void = 0;
};

class ShaderLibrary {
public:
  ShaderLibrary() = default;

public:
  auto add(const std::string& /* name */, Shared<GPUShader>) -> void;
  auto get(const std::string& /* name */) -> Shared<GPUShader>;

private:
  std::unordered_map<std::string, Shared<GPUShader>> m_shaders;
};

} // namespace zod
