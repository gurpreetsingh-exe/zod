#pragma once

namespace zod {

enum class GPUDataType {
  Int,
  Float,
  Vec2,
  Vec3,
  Vec4,
  Mat3,
  Mat4,
};

auto gpu_sizeof(GPUDataType) -> usize;

} // namespace zod
