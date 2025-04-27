#include "engine/mesh.hh"
#include "core/platform/platform.hh"

namespace zod {

static u32 magic = 0X234D5A2E;
static Point g_points[] = {
  { .P = { -1.0f, 1.0f, 1.0f } },   { .P = { -1.0f, -1.0f, 1.0f } },
  { .P = { 1.0f, 1.0f, 1.0f } },    { .P = { 1.0f, -1.0f, 1.0f } },
  { .P = { 1.0f, 1.0f, -1.0f } },   { .P = { 1.0f, -1.0f, -1.0f } },
  { .P = { -1.0f, -1.0f, -1.0f } }, { .P = { -1.0f, 1.0f, -1.0f } },
};
static vec3 g_normals[] = {
  { 0.0f, 0.0f, 1.0f },  { 0.0f, 0.0f, 1.0f },  { 1.0f, 0.0f, 0.0f },
  { 1.0f, 0.0f, 0.0f },  { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f },
  { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f },
  { 0.0f, 1.0f, 0.0f },  { -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f },
};
static Prim g_prims[] = {
  { .points = { 0, 1, 2 } }, { .points = { 1, 3, 2 } },
  { .points = { 4, 2, 5 } }, { .points = { 2, 3, 5 } },
  { .points = { 5, 3, 6 } }, { .points = { 3, 1, 6 } },
  { .points = { 6, 7, 5 } }, { .points = { 7, 4, 5 } },
  { .points = { 7, 0, 4 } }, { .points = { 0, 2, 4 } },
  { .points = { 6, 1, 7 } }, { .points = { 1, 0, 7 } },
};
static SubMesh g_submeshes[] = {
  { .mat = {}, .offset = 0, .size = 36 },
};

auto Mesh::cube() -> SharedPtr<Mesh> {
  auto mesh = shared<Mesh>();
  *mesh = {
    .points = Span<Point>(g_points, 8),
    .normals = Span<vec3>(g_normals, 12),
    .prims = Span<Prim>(g_prims, 12),
    .submeshes = Span<SubMesh>(g_submeshes, 1),
  };
  return mesh;
}

auto Mesh::write(Archive& ar) -> void {
  ar.copy((u8*)&magic, sizeof(u32));
  auto size = points.size();
  ar.copy((u8*)&size, sizeof(usize));
  ar.copy((u8*)points.data(), points.size_bytes());

  size = normals.size();
  ar.copy((u8*)&size, sizeof(usize));
  ar.copy((u8*)normals.data(), normals.size_bytes());

  size = uvs.size();
  ar.copy((u8*)&size, sizeof(usize));
  ar.copy((u8*)uvs.data(), uvs.size_bytes());

  size = prims.size();
  ar.copy((u8*)&size, sizeof(usize));
  ar.copy((u8*)prims.data(), prims.size_bytes());

  size = submeshes.size();
  ar.copy((u8*)&size, sizeof(usize));
  ar.copy((u8*)submeshes.data(), submeshes.size_bytes());
}

auto Mesh::read(const fs::path& path) -> void {
  u8* mapping = (u8*)memory_map(path).page;
  if (*(u32*)mapping != magic) {
    eprintln("invalid magic bits found");
  }

  auto offset = 4;
  points =
      Span((Point*)&mapping[offset + sizeof(usize)], *(usize*)&mapping[offset]);
  offset += points.size_bytes() + sizeof(usize);

  normals =
      Span((vec3*)&mapping[offset + sizeof(usize)], *(usize*)&mapping[offset]);
  offset += normals.size_bytes() + sizeof(usize);

  uvs =
      Span((vec2*)&mapping[offset + sizeof(usize)], *(usize*)&mapping[offset]);
  offset += uvs.size_bytes() + sizeof(usize);

  prims =
      Span((Prim*)&mapping[offset + sizeof(usize)], *(usize*)&mapping[offset]);
  offset += prims.size_bytes() + sizeof(usize);

  submeshes = Span((SubMesh*)&mapping[offset + sizeof(usize)],
                   *(usize*)&mapping[offset]);

  fmt::println("Points: {}", points.size());
  fmt::println("Normals: {}", normals.size());
  fmt::println("UVs: {}", uvs.size());
  fmt::println("Triangles: {}", prims.size());
  fmt::println("SubMeshes: {}", submeshes.size());
}

} // namespace zod
