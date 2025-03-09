#include "engine/mesh.hh"

namespace zod {

static u32 magic = 0X234D5A2E;

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
  ar.copy((u8*)&magic, sizeof(u32));
  auto size = points.size();
  ar.copy((u8*)&size, sizeof(usize));
  ar.copy((u8*)points.data(), points.size() * sizeof(Point));

  size = normals.size();
  ar.copy((u8*)&size, sizeof(usize));
  ar.copy((u8*)normals.data(), normals.size() * sizeof(vec3));

  size = prims.size();
  ar.copy((u8*)&size, sizeof(usize));
  ar.copy((u8*)prims.data(), prims.size() * sizeof(Prim));
}

auto Mesh::read(const fs::path& path) -> void {
  // no error checking pog
  auto* f = std::fopen(path.string().c_str(), "rb");
  std::fseek(f, 0L, SEEK_END);
  auto file_size = std::ftell(f);
  std::rewind(f);

  u32 magic_check = 0;
  std::fread(&magic_check, sizeof(u32), 1, f);
  ZASSERT(magic_check == magic);

  usize size = 0;
  std::fread(&size, sizeof(usize), 1, f);
  points.resize(size);
  std::fread(points.data(), sizeof(Point), size, f);

  std::fread(&size, sizeof(usize), 1, f);
  normals.resize(size);
  std::fread(normals.data(), sizeof(vec3), size, f);

  std::fread(&size, sizeof(usize), 1, f);
  prims.resize(size);
  std::fread(prims.data(), sizeof(Prim), size, f);
}

} // namespace zod
