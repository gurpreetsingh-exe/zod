#pragma once

#include "core/event.hh"

namespace zod {

inline bool g_input_state[Key::Last] = {};
inline vec2 g_last_mouse_pos = {};

struct Input {
  static auto is_key_pressed(int) -> bool;
  static auto is_mouse_button_pressed(int) -> bool;
  static auto get_mouse_pos() -> vec2;
  static auto set_mouse_pos(vec2) -> void;
};

template <typename... Keys>
auto any_key(Keys&&... keys) -> bool {
  return (... or g_input_state[keys]);
}

} // namespace zod
