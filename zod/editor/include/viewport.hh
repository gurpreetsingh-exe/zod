#pragma once

#include "backend.hh"
#include "widgets/panel.hh"

namespace zod {

class Viewport : public SPanel {
public:
  Viewport();
  auto update(Shared<GPUBatch>) -> void;

private:
  auto draw_imp(Geometry&) -> void override {
    update(ZCtxt::get().get_batch());
  }
  auto on_event_imp(Event&) -> void override;
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
