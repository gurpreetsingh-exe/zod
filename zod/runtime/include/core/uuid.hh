#pragma once

#include <uuid_v4.h>

namespace zod {

class UUID {
public:
  explicit UUID(u8 hash[16]) : m_inner(hash) {}
  explicit UUID(const String& id) : m_inner(UUIDv4::UUID::fromStrFactory(id)) {}
  UUID() : m_inner(s_generator.getUUID()) {}

  static auto null() -> UUID {
    static u8 uuid_null_init[16] = { 0 };
    return UUID(uuid_null_init);
  }

  auto hash() const -> usize { return m_inner.hash(); }
  friend auto operator==(const UUID& us, const UUID& other) -> bool {
    return us.m_inner == other.m_inner;
  }

  auto to_string() const -> String { return m_inner.str(); }

private:
  UUIDv4::UUID m_inner;
  inline static UUIDv4::UUIDGenerator<std::mt19937_64> s_generator;
};

} // namespace zod

namespace std {
template <>
struct hash<zod::UUID> {
  auto operator()(const zod::UUID& uuid) const -> zod::usize {
    return uuid.hash();
  }
};
} // namespace std
