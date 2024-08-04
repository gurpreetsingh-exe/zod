#pragma once

#include "config.hh"

namespace zod {

auto config_from_args(i32 argc, char** argv) -> Config;

} // namespace zod
