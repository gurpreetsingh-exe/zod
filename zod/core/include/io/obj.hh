#pragma once

#include "mesh/mesh.hh"

namespace zod {

auto load_obj(const fs::path& filepath) -> Mesh*;

}
