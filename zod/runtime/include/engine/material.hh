#pragma once

#include "core/uuid.hh"

namespace zod {

struct Material {
  UUID color_texture = UUID::null();
  UUID normal_texture = UUID::null();
  UUID roughness_texture = UUID::null();
};

} // namespace zod
