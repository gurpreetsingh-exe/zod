#include "core/platform.hh"

namespace zod {

auto find_config_dir() -> fs::path {
  auto exe_dir = get_exe_path().parent_path();
  auto config = exe_dir / "config/";
  if (fs::exists(config)) {
    return config;
  }

  config = exe_dir.parent_path() / "config/";
  if (fs::exists(config)) {
    return config;
  }

  UNREACHABLE();
}

} // namespace zod
