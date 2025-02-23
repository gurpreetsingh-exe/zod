#pragma once

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <format>
#include <functional>
#include <ranges>
#include <typeinfo>
#include <unordered_map>

#include "lazy.hh"
#include "macros.hh"
#include "math.hh"

#include "core/containers/string.hh"
#include "core/containers/vector.hh"
#include "core/debug/panic.hh"
#include "core/memory/memory.hh"
#include "core/misc.hh"

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

} // namespace zod
