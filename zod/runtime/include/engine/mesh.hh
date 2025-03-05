#pragma once

namespace zod {

struct Point {
  vec3 P;
  auto operator==(const Point& other) const -> bool { return P == other.P; }
};

struct Prim {
  Vector<u32> points;
};

struct Mesh {
  Vector<Point> points;
  Vector<vec3> normals;
  Vector<Prim> prims;

  static auto cube() -> SharedPtr<Mesh> {
    auto mesh = shared<Mesh>();
    *mesh = {
      .points =
          Vector<Point> {
              { .P = { -1.0f, 1.0f, 1.0f } },
              { .P = { -1.0f, -1.0f, 1.0f } },
              { .P = { 1.0f, 1.0f, 1.0f } },
              { .P = { 1.0f, -1.0f, 1.0f } },
              { .P = { 1.0f, 1.0f, -1.0f } },
              { .P = { 1.0f, -1.0f, -1.0f } },
              { .P = { -1.0f, -1.0f, -1.0f } },
              { .P = { -1.0f, 1.0f, -1.0f } },
          },
      .normals =
          Vector<vec3> {
              { 0.0f, 0.0f, 1.0f },
              { 0.0f, 0.0f, 1.0f },
              { 1.0f, 0.0f, 0.0f },
              { 1.0f, 0.0f, 0.0f },
              { 0.0f, -1.0f, 0.0f },
              { 0.0f, -1.0f, 0.0f },
              { 0.0f, 0.0f, -1.0f },
              { 0.0f, 0.0f, -1.0f },
              { 0.0f, 1.0f, 0.0f },
              { 0.0f, 1.0f, 0.0f },
              { -1.0f, 0.0f, 0.0f },
              { -1.0f, 0.0f, 0.0f },
          },
      .prims =
          Vector<Prim> {
              { .points = { 0, 1, 2 } },
              { .points = { 1, 3, 2 } },
              { .points = { 4, 2, 5 } },
              { .points = { 2, 3, 5 } },
              { .points = { 5, 3, 6 } },
              { .points = { 3, 1, 6 } },
              { .points = { 6, 7, 5 } },
              { .points = { 7, 4, 5 } },
              { .points = { 7, 0, 4 } },
              { .points = { 0, 2, 4 } },
              { .points = { 6, 1, 7 } },
              { .points = { 1, 0, 7 } },
          },
    };
    return mesh;
  }
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
