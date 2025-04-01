#pragma once

namespace zod {

enum class GPUTextureType {
  Texture1D,
  Texture2D,
  Texture3D,
  TextureCube,
  TextureArray,
};

enum class GPUTextureFormat {
  RGBA8,
  RGB32F,
  RGBA32F,
  R32UI,
  Red,
};

enum class GPUTextureData {
  UByte,
  Float,
};

enum class GPUTextureWrap {
  Repeat,
  Clamp,
};

struct GPUTextureCreateInfo {
  i32 width;
  i32 height;
  GPUTextureType type = GPUTextureType::Texture2D;
  GPUTextureFormat format = GPUTextureFormat::RGBA8;
  GPUTextureData data = GPUTextureData::UByte;
  GPUTextureWrap wrap = GPUTextureWrap::Repeat;
  fs::path path = {};
  void* pixels = nullptr;
  usize layers = 0;
  usize mips = 0;
};

class GPUTexture {
protected:
  GPUTextureCreateInfo m_info;
  GPUTexture(GPUTextureCreateInfo info) : m_info(info) {}

public:
  virtual ~GPUTexture() = default;
  virtual auto bind() -> void = 0;
  virtual auto unbind() -> void = 0;
  virtual auto resize(i32, i32) -> void = 0;
  virtual auto get_id() -> void* = 0;
  virtual auto blit(f32, f32, f32, f32, const void* /* pixels */,
                    usize /* layers */ = 0) -> void = 0;
  virtual auto generate_mipmap() -> void = 0;
  auto get_size() -> vec2 { return { f32(m_info.width), f32(m_info.height) }; }
};

} // namespace zod
