#pragma once

#include "core/platform/macros.hh"

namespace zod {

enum class Platform {
  Windows,
  Linux,
};

constexpr auto get_platform() -> Platform;
auto get_exe_path() -> fs::path;

} // namespace zod
