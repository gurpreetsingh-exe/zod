#pragma once

#include "core/serialize/archive.hh"

namespace zod {

struct Point {
  vec3 P;
  auto operator==(const Point& other) const -> bool { return P == other.P; }
};

struct Prim {
  u32 points[3];
};

struct Mesh {
  Vector<Point> points;
  Vector<vec3> normals;
  Vector<Prim> prims;

  static auto cube() -> SharedPtr<Mesh>;
  auto write(Archive&) -> void;
};

} // namespace zod

namespace std {
template <>
struct hash<zod::Point> {
  auto operator()(zod::Point const& point) const -> zod::usize {
    return hash<zod::vec3>()(point.P);
  }
};
} // namespace std

FMT(zod::Prim, "[{}, {}, {}]", v.points[0], v.points[1], v.points[2]);
FMT(zod::Point, "{}", v.P);
