#pragma once

#include "types.hh"

#define X(name) extern const char* g_##name;
#include "shaders.def"
#undef X

namespace zod {

enum {
  GPU_VERTEX_SHADER_BIT = 1 << 0,
  GPU_FRAGMENT_SHADER_BIT = 1 << 1,
  GPU_COMPUTE_SHADER_BIT = 1 << 2,
};

class GPUShaderCreateInfo {
public:
  const char* name;

  struct VertexIn {
    u32 index;
    GPUDataType type;
    const char* name;
  };
  std::vector<VertexIn> inputs = {};

  struct FragmentOut {
    u32 index;
    GPUDataType type;
    const char* name;
  };
  std::vector<FragmentOut> outputs = {};

  struct Varying {
    u32 index;
    GPUDataType type;
    const char* name;
  };
  std::vector<Varying> varyings = {};

  GPUShaderCreateInfo(const char* name_) : name(name_) {}
  auto vertex_in(VertexIn) -> GPUShaderCreateInfo&;
  auto fragment_out(FragmentOut) -> GPUShaderCreateInfo&;
  auto varying(Varying) -> GPUShaderCreateInfo&;
  auto vertex_source(const char*) -> GPUShaderCreateInfo&;
  auto fragment_source(const char*) -> GPUShaderCreateInfo&;
  auto compute_source(const char*) -> GPUShaderCreateInfo&;
  auto shader_bits() const -> u8 { return m_shader_bits; }
  auto get_vertex_source() -> const char* { return m_vertex_source; }
  auto get_fragment_source() -> const char* { return m_fragment_source; }
  auto get_compute_source() -> const char* { return m_compute_source; }

private:
  u8 m_shader_bits = 0;
  const char* m_vertex_source = nullptr;
  const char* m_fragment_source = nullptr;
  const char* m_compute_source = nullptr;
};

class GPUShader {
public:
  std::string name;

public:
  GPUShader(GPUShaderCreateInfo info) : name(info.name) {}
  virtual ~GPUShader() = default;
  virtual auto bind() -> void = 0;
  virtual auto unbind() -> void = 0;
  virtual auto dispatch(u32 /* x */, u32 /* y */, u32 /* z */) -> void = 0;
  virtual auto uniform_float(const std::string&, const f32*,
                             usize /* size */ = 1) -> void = 0;
  virtual auto uniform_uint(const std::string&, const u32*,
                            usize /* size */ = 1) -> void = 0;
  virtual auto uniform_int(const std::string&, const i32*, usize /* size */ = 1)
      -> void = 0;

private:
  virtual auto
  init_vertex_and_fragment_shader(const char* /* vertex_source */,
                                  const char* /* fragment_source */)
      -> void = 0;
  virtual auto init_compute_shader(const char* /* source */) -> void = 0;
  virtual auto compile(GPUShaderCreateInfo) -> void = 0;
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
