#pragma once

#include "core/serialize/archive.hh"
#include "core/span.hh"

namespace zod {

struct Point {
  vec3 P;
  auto operator==(const Point& other) const -> bool { return P == other.P; }
};

struct Prim {
  u32 points[3];
};

struct Material {
  usize color_texture = -1ZU;
  usize normal_texture = -1ZU;
  usize roughness_texture = -1ZU;
};

struct SubMesh {
  Material mat;
  usize offset;
  usize size;
};

struct Mesh {
  Span<Point> points;
  Span<vec3> normals;
  Span<vec2> uvs;
  Span<Prim> prims;

  Span<SubMesh> submeshes;

  static auto cube() -> SharedPtr<Mesh>;
  auto write(Archive&) -> void;
  auto read(const fs::path&) -> void;
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
