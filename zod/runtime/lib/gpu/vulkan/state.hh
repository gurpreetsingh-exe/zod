#pragma once

#include "gpu/state.hh"

namespace zod {

class VKState : public GPUState {
  auto set_depth_test(Depth) -> void override {}
  auto set_blend(Blend) -> void override {}
  auto draw_immediate(usize /* number_of_points */) -> void override {}
};

} // namespace zod
