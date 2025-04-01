#pragma once

#include "core/platform/macros.hh"

namespace zod {

enum class Platform {
  Windows,
  Linux,
};

struct Mapping {
  void* page;
  usize size;

  auto operator[](usize idx) -> void* {
    ZASSERT(idx < size);
    return (u8*)page + idx;
  }
};

constexpr auto get_platform() -> Platform;
auto get_exe_path() -> fs::path;
auto memory_map(const fs::path&) -> Mapping;
auto memory_unmap(Mapping) -> void;

} // namespace zod
