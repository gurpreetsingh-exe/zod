#pragma once

namespace zod {

enum class Depth {
  None,
  Less,
  LessEqual,
  Equal,
  Greater,
  GreaterEqual,
  Always,
};

enum class Blend {
  None,
  Alpha,
};

class GPUState {
public:
  virtual ~GPUState() = default;
  static auto get() -> GPUState&;

public:
  virtual auto set_depth_test(Depth) -> void = 0;
  virtual auto set_blend(Blend) -> void = 0;
  virtual auto draw_immediate(usize /* number_of_points */) -> void = 0;
};

} // namespace zod
