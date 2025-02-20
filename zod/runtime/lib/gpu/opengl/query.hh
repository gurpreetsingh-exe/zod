#pragma once

#include <glad/glad.h>

#include "gpu/query.hh"

namespace zod {

class GLQuery : public GPUQuery {
public:
  GLQuery();

public:
  auto begin() -> void override;
  auto end() -> void override;
  auto get_time() -> f32 override;

private:
  GLuint m_id[2];
  GLint64 m_start;
  GLint64 m_end;
};

} // namespace zod
