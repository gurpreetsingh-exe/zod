#pragma once

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <execinfo.h>
#include <experimental/source_location>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "lazy.hh"
#include "macros.hh"
#include "math.hh"

namespace fs = std::filesystem;
namespace chr = std::chrono;
namespace ph = std::placeholders;
namespace rng = std::ranges;

namespace zod {

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using usize = size_t;
using f32 = float;
using f64 = double;
using loc = std::experimental::source_location;

template <typename T>
using Option = std::optional<T>;

template <typename T>
using Shared = std::shared_ptr<T>;

template <typename T>
using Unique = std::unique_ptr<T>;

template <typename T>
using Weak = std::weak_ptr<T>;

template <typename T, typename... Args>
auto shared(Args&&... args) -> Shared<T> {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
auto unique(Args&&... args) -> Unique<T> {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

/// Used in ADDR() macro
template <typename T>
struct Addr {
  Addr(T v) : inner(v) {}
  auto operator*() const -> const T* { return &inner; }
  T inner;
};

static constexpr bool backtrace_addr2line = true;
static constexpr std::string_view backtrace_addr2line_options = "-p -C -f";
static constexpr bool backtrace_llvm_symbolizer = false;
static constexpr std::string_view backtrace_llvm_symbolizer_options =
    "-s -p -C -i --color --output-style=GNU";

inline auto print_backtrace() -> void {
  constexpr usize size = 128;
  void* trace[size];
  int n = backtrace(trace, size);
  char** strings = backtrace_symbols(&trace[0], size);
  // for (usize i = 0; i < (usize)size; ++i) {
  //   fmt::println(stderr, "    #{:2} {}", i, strings[i]);
  // }
  int skip_value = 1;

  if constexpr (backtrace_addr2line) {
    for (int i = skip_value; i < n; ++i) {
      trace[i] = (void*)((std::uintptr_t)trace[i] - 1);
    }

    std::span<void*> trace_view { &trace[skip_value], &trace[n] };

    std::string command = fmt::format(
        "addr2line {} -e {} {}", backtrace_addr2line_options,
        fs::canonical("/proc/self/exe").native(), fmt::join(trace_view, " "));
    std::system(command.data());
  }
}

template <typename... T>
auto __panic(loc loc, fmt::format_string<T...> fmt, T&&... args) {
  fmt::print(stderr, "panic at {}:{}", loc.file_name(), loc.line());
  fmt::println(stderr, fmt, args...);
  fmt::println(stderr, "BACKTRACE:");
  print_backtrace();
  exit(1);
}

inline auto demangle(const std::string& name) -> std::string {
  int status = -4;
  std::unique_ptr<char, void (*)(void*)> res {
    abi::__cxa_demangle(name.c_str(), NULL, NULL, &status), std::free
  };

  return (status == 0) ? res.get() : name;
}

} // namespace zod
