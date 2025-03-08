#include "engine/mesh.hh"

namespace zod {

auto Mesh::cube() -> SharedPtr<Mesh> {
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

auto Mesh::write(Archive& ar) -> void {
  ar.copy((u8*)points.data(), points.size() * sizeof(Point));
  ar.copy((u8*)normals.data(), normals.size() * sizeof(vec3));
  ar.copy((u8*)prims.data(), prims.size() * sizeof(Prim));
}

} // namespace zod
