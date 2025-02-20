#pragma once

#include <uuid_v4.h>

namespace zod {

using uuid = UUIDv4::UUID;

constexpr u8 uuid_null_init[16] = { 0 };

inline auto uuid0() -> uuid { return uuid(uuid_null_init); }

} // namespace zod
