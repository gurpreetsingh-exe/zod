#pragma once

#include "gpu/batch.hh"
#include "vk_common.hh"

namespace zod {

class VKBatch : public GPUBatch {
public:
  VKBatch(const Vector<GPUBufferLayout>&, const Vector<u32>& indices = {});
  ~VKBatch();

public:
  auto draw(SharedPtr<GPUShader>) -> void override;
  auto draw(SharedPtr<GPUShader>, usize) -> void override;
  auto draw_instanced(SharedPtr<GPUShader>, usize /* instance_count */)
      -> void override;
  auto draw_lines(SharedPtr<GPUShader>) -> void;

private:
  usize m_elements;
};

} // namespace zod
