#include "engine/mesh.hh"
#include "core/platform/platform.hh"

namespace zod {

static u32 magic = 0X234D5A2E;
static Point g_points[] = {
  { .P = { -1.0f, -1.0f, 1.0f } },  { .P = { 1.0f, -1.0f, 1.0f } },
  { .P = { 1.0f, 1.0f, 1.0f } },    { .P = { -1.0f, 1.0f, 1.0f } },

  { .P = { -1.0f, 1.0f, -1.0f } },  { .P = { 1.0f, 1.0f, -1.0f } },
  { .P = { 1.0f, -1.0f, -1.0f } },  { .P = { -1.0f, -1.0f, -1.0f } },

  { .P = { 1.0f, -1.0f, -1.0f } },  { .P = { 1.0f, 1.0f, -1.0f } },
  { .P = { 1.0f, 1.0f, 1.0f } },    { .P = { 1.0f, -1.0f, 1.0f } },

  { .P = { -1.0f, 1.0f, -1.0f } },  { .P = { -1.0f, -1.0f, -1.0f } },
  { .P = { -1.0f, -1.0f, 1.0f } },  { .P = { -1.0f, 1.0f, 1.0f } },

  { .P = { 1.0f, 1.0f, -1.0f } },   { .P = { -1.0f, 1.0f, -1.0f } },
  { .P = { -1.0f, 1.0f, 1.0f } },   { .P = { 1.0f, 1.0f, 1.0f } },

  { .P = { -1.0f, -1.0f, -1.0f } }, { .P = { 1.0f, -1.0f, -1.0f } },
  { .P = { 1.0f, -1.0f, 1.0f } },   { .P = { -1.0f, -1.0f, 1.0f } },
};
static vec3 g_normals[] = {
  { 0.0f, 0.0f, 1.0f },  { 0.0f, 0.0f, 1.0f },
  { 0.0f, 0.0f, 1.0f },  { 0.0f, 0.0f, 1.0f },

  { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },
  { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },

  { 1.0f, 0.0f, 0.0f },  { 1.0f, 0.0f, 0.0f },
  { 1.0f, 0.0f, 0.0f },  { 1.0f, 0.0f, 0.0f },

  { -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f },
  { -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f },

  { 0.0f, 1.0f, 0.0f },  { 0.0f, 1.0f, 0.0f },
  { 0.0f, 1.0f, 0.0f },  { 0.0f, 1.0f, 0.0f },

  { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f },
  { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f },
};
static vec2 g_uvs[] = {
  { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
  { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
  { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
  { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
  { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
  { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
};
static Prim g_prims[] = {
  { .points = { 0, 1, 2 } },    { .points = { 2, 3, 0 } },
  { .points = { 4, 5, 6 } },    { .points = { 6, 7, 4 } },
  { .points = { 8, 9, 10 } },   { .points = { 10, 11, 8 } },
  { .points = { 12, 13, 14 } }, { .points = { 14, 15, 12 } },
  { .points = { 16, 17, 18 } }, { .points = { 18, 19, 16 } },
  { .points = { 20, 21, 22 } }, { .points = { 22, 23, 20 } },
};
auto Mesh::cube() -> SharedPtr<Mesh> {
  auto mesh = shared<Mesh>();
  *mesh = {
    .points = Span<Point>(g_points, 24),
    .normals = Span<vec3>(g_normals, 24),
    .uvs = Span<vec2>(g_uvs, 24),
    .prims = Span<Prim>(g_prims, 12),
    .submeshes = { { .offset = 0, .size = 36 } },
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
  for (const auto& submesh : submeshes) {
    auto material = submesh.mat.to_string();
    auto material_size = material.size();
    ar.copy((u8*)&material_size, sizeof(usize));
    ar.copy((u8*)material.data(), material_size);
    ar.copy((u8*)&submesh.offset, sizeof(usize));
    ar.copy((u8*)&submesh.size, sizeof(usize));
  }
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

  auto number_submeshes = *(usize*)&mapping[offset];
  offset += sizeof(usize);
  submeshes.resize(number_submeshes);
  for (auto i = 0ZU; i < number_submeshes; ++i) {
    auto material_size = *(usize*)&mapping[offset];
    offset += sizeof(usize);
    auto material = String((char*)&mapping[offset], material_size);
    offset += material_size;
    submeshes[i].mat = UUID(material);
    submeshes[i].offset = *(usize*)&mapping[offset];
    offset += sizeof(usize);
    submeshes[i].size = *(usize*)&mapping[offset];
    offset += sizeof(usize);
  }

  fmt::println("Points: {}", points.size());
  fmt::println("Normals: {}", normals.size());
  fmt::println("UVs: {}", uvs.size());
  fmt::println("Triangles: {}", prims.size());
  fmt::println("SubMeshes: {}", submeshes.size());
}

} // namespace zod
