#pragma once

#include "base/math.hh"

namespace zod {

enum class GPUTextureType {
  Texture1D,
  Texture2D,
  Texture3D,
  TextureCube,
};

class GPUTexture {
protected:
  i32 m_width = 0;
  i32 m_height = 0;
  GPUTextureType m_type;

  GPUTexture(GPUTextureType type, i32 width, i32 height)
      : m_width(width), m_height(height), m_type(type) {}

public:
  virtual ~GPUTexture() = default;
  virtual auto bind() -> void = 0;
  virtual auto unbind() -> void = 0;
  virtual auto resize(i32, i32) -> void = 0;
  virtual auto get_id() -> void* = 0;
  auto get_size() -> vec2 { return { f32(m_width), f32(m_height) }; }
};

} // namespace zod
