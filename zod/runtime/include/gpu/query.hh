#pragma once

namespace zod {

class GPUQuery {
public:
  ~GPUQuery() = default;
  virtual auto begin() -> void = 0;
  virtual auto end() -> void = 0;
  virtual auto get_time() -> f32 = 0;
};

} // namespace zod
