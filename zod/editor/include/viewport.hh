#pragma once

#include "gpu/backend.hh"
#include "widgets/panel.hh"

namespace zod {

class Viewport : public SPanel {
public:
  Viewport();

private:
  auto draw_imp(Geometry&) -> void override;
  auto on_event_imp(Event&) -> void override;
  auto draw_cubemap() -> void;
  auto draw_grid() -> void;

public:
  auto gbuffer() -> bool& { return m_gbuffer_display; }
  auto gbuffer_slot() -> i32& { return m_gbuffer_slot; }

private:
  f32 m_width;
  f32 m_height;
  SharedPtr<GPUShader> m_grid_shader;

  bool m_gbuffer_display = false;
  i32 m_gbuffer_slot = 0;
};

} // namespace zod
