#pragma once

#include <glad/glad.h>

#include "../batch.hh"

namespace zod {

class GLBatch : public GPUBatch {
public:
  GLBatch(const std::vector<GPUBufferLayout>&,
          const std::vector<u32>& indices = {});
  ~GLBatch();

public:
  auto draw(Shared<GPUShader>) -> void override;
  auto draw(Shared<GPUShader>, usize) -> void override;
  auto draw_instanced(Shared<GPUShader>, usize /* instance_count */)
      -> void override;
  auto draw_lines(Shared<GPUShader>) -> void;
  auto draw_lines(Shared<GPUShader>, usize) -> void override;

private:
  usize m_elements;
  GLuint m_id;
};

} // namespace zod
