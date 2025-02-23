#pragma once

#include <glad/glad.h>

#include "gpu/batch.hh"

namespace zod {

class GLBatch : public GPUBatch {
public:
  GLBatch(const Vector<GPUBufferLayout>&, const Vector<u32>& indices = {});
  ~GLBatch();

public:
  auto draw(SharedPtr<GPUShader>) -> void override;
  auto draw(SharedPtr<GPUShader>, usize) -> void override;
  auto draw_instanced(SharedPtr<GPUShader>, usize /* instance_count */)
      -> void override;
  auto draw_indirect(SharedPtr<GPUShader>) -> void;
  auto upload_indirect(const void*, usize) -> void;
  auto draw_lines(SharedPtr<GPUShader>) -> void;
  auto draw_lines(SharedPtr<GPUShader>, usize) -> void override;

private:
  usize m_elements;
  GLuint m_id;
  GLuint m_indirect;
  usize m_indirect_size;
};

} // namespace zod
