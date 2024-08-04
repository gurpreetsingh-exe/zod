#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace zod {

struct Point {
  glm::vec3 P;
  auto operator==(const Point& other) const -> bool { return P == other.P; }
};

struct Prim {
  std::vector<u32> points;
};

struct Mesh {
  std::vector<Point> points;
  std::vector<Prim> prims;
};

} // namespace zod

namespace std {
template <>
struct hash<zod::Point> {
  auto operator()(zod::Point const& point) const -> zod::usize {
    return hash<glm::vec3>()(point.P);
  }
};
} // namespace std

template <>
struct fmt::formatter<zod::Prim> {
  constexpr auto parse(format_parse_context& ctx)
      -> format_parse_context::iterator {
    return ctx.begin();
  }

  auto format(zod::Prim prim, format_context& ctx) const
      -> format_context::iterator {
    return fmt::format_to(ctx.out(), "[{}]", fmt::join(prim.points, ", "));
  }
};

template <>
struct fmt::formatter<zod::Point> {
  constexpr auto parse(format_parse_context& ctx)
      -> format_parse_context::iterator {
    return ctx.begin();
  }

  auto format(zod::Point point, format_context& ctx) const
      -> format_context::iterator {
    return fmt::format_to(ctx.out(), "{{ {}, {}, {} }}", point.P.x, point.P.y,
                          point.P.z);
  }
};
