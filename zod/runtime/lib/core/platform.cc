#ifdef PLATFORM_WINDOWS
#  include <windows.h>
#else
#  include <limits.h>
#  include <unistd.h>
#endif

#include "core/platform/platform.hh"

namespace zod {

constexpr auto get_platform() -> Platform {
#ifdef PLATFORM_WINDOWS
  return Platform::Windows;
#else
  return Platform::Linux;
#endif
}

auto get_exe_path() -> fs::path {
#ifdef PLATFORM_WINDOWS
  wchar_t path[MAX_PATH] = { 0 };
  GetModuleFileNameW(NULL, path, MAX_PATH);
  return path;
#else
  char result[PATH_MAX] = {};
  auto count = readlink("/proc/self/exe", result, PATH_MAX);
  return String(result, (count > 0) ? count : 0);
#endif
}

} // namespace zod
