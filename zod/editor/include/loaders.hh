#pragma once

namespace zod {

struct Mesh;

auto load_obj(const fs::path&) -> SharedPtr<Mesh>;
auto loadGLTF(const fs::path&) -> SharedPtr<Mesh>;

} // namespace zod
