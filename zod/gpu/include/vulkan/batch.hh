#pragma once

#include "../batch.hh"
#include "vk_common.hh"

namespace zod {

class VKBatch : public GPUBatch {
public:
  VKBatch(const std::vector<GPUBufferLayout>&,
          const std::vector<u32>& indices = {});
  ~VKBatch();

public:
  auto draw(Shared<GPUShader>) -> void override;
  auto draw(Shared<GPUShader>, usize) -> void override;
  auto draw_instanced(Shared<GPUShader>, usize /* instance_count */)
      -> void override;
  auto draw_lines(Shared<GPUShader>) -> void;

private:
  usize m_elements;
};

} // namespace zod
