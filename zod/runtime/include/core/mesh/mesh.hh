#pragma once

namespace zod {

struct Point {
  vec3 P;
  auto operator==(const Point& other) const -> bool { return P == other.P; }
};

struct Prim {
  std::vector<u32> points;
};

struct Mesh {
  std::vector<Point> points;
  std::vector<vec3> normals;
  std::vector<Prim> prims;
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

FMT(zod::Prim, "[{}]", fmt::join(v.points, ", "));
FMT(zod::Point, "{}", v.P);
