#pragma once

#include "gpu/state.hh"

namespace zod {

class GLState : public GPUState {
public:
  GLState();

public:
  auto set_depth_test(Depth) -> void override;
  auto set_blend(Blend) -> void override;

  // TODO: a bit sus to put this here
  auto draw_immediate(usize /* number_of_points */) -> void override;
};

} // namespace zod
