#pragma once

namespace zod {

struct Input {
  static auto is_key_pressed(int) -> bool;
  static auto is_mouse_button_pressed(int) -> bool;
  static auto get_mouse_pos() -> std::pair<f32, f32>;
};

} // namespace zod
