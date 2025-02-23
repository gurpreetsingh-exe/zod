#pragma once

namespace zod {

struct Mesh;

auto load_obj(const fs::path& filepath) -> Mesh*;

} // namespace zod
