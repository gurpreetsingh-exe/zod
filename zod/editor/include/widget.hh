#pragma once

namespace zod {

class Widget {
public:
  virtual ~Widget() = default;
  virtual auto calculate(f32& /* x */, f32& /* y */, f32& /* width */,
                         f32& /* height */) -> void = 0;
  // virtual auto generate(GMesh&) -> void = 0;
};

} // namespace zod
