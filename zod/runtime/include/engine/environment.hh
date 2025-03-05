#pragma once

#include "core/uuid.hh"
#include "engine/property.hh"

namespace zod {

enum LightingMode : int {
  SolidColor = 0,
  Texture,
};

struct Environment {
  LightingMode mode = LightingMode::Texture;
  /// Color of the background, when mode is LightingMode::SolidColor
  Property color = { "Color", { 0.1f, 0.1f, 0.1f }, PROP_SUBTYPE_COLOR };
  /// Path to the HDRI image, when mode is LightingMode::Texture
  Property hdr = { "Path", new char[STRING_PROP_MAX_SIZE],
                   PROP_SUBTYPE_FILEPATH };
};

} // namespace zod
