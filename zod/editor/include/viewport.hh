#pragma once

#include "gpu/backend.hh"
#include "widgets/panel.hh"

namespace zod {

class Viewport : public SPanel {
public:
  Viewport();
  auto update(SharedPtr<GPUBatch>) -> void;

private:
  auto draw_imp(Geometry&) -> void override {
    update(Editor::get().get_batch());
  }
  auto on_event_imp(Event&) -> void override;
  auto draw_cubemap() -> void;
  auto draw_grid() -> void;
  auto draw_axes() -> void;

private:
  f32 m_width;
  f32 m_height;
  SharedPtr<GPUShader> m_shader;
  SharedPtr<GPUTexture> m_cubemap = nullptr;
  SharedPtr<GPUBatch> m_cubemap_batch;
  SharedPtr<GPUShader> m_cubemap_shader;
  SharedPtr<GPUShader> m_grid_shader;
};

} // namespace zod
