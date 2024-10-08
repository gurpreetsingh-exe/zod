#pragma once

#include "backend.hh"
#include "widgets/panel.hh"

namespace zod {

class Viewport : public Panel {
public:
  Viewport();
  auto update(Shared<GPUBatch>) -> void;

  /// get batch in here somehow
  auto draw(Geometry&) -> void override {}

private:
  auto draw_cubemap() -> void;
  auto draw_grid() -> void;
  auto draw_axes() -> void;

private:
  f32 m_width;
  f32 m_height;
  Shared<GPUShader> m_shader;
  Shared<GPUBatch> m_cubemap_batch;
  Shared<GPUShader> m_cubemap_shader;
  Shared<GPUShader> m_grid_shader;
};

} // namespace zod
