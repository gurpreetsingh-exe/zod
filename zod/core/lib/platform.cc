#ifdef _WIN32
#include <windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

#include "platform.hh"

namespace zod {

auto open_dialog(const fs::path& default_path) -> std::string {
  char buf[128] = {};
#if defined(_WIN32) || defined(_WIN64)
#error "not implemented"
#else
  auto* f = popen(
      fmt::format(
          "zenity --title=\"Select File\" --file-selection --filename=\"{}\"",
          default_path.string())
          .c_str(),
      "r");
  fgets(buf, 128, f);
  buf[strlen(buf) - 1] = '\0';
#endif
  return buf;
}

auto get_exe_path() -> fs::path {
#ifdef _WIN32
  wchar_t path[MAX_PATH] = { 0 };
  GetModuleFileNameW(NULL, path, MAX_PATH);
  return path;
#else
  char result[PATH_MAX] = {};
  auto count = readlink("/proc/self/exe", result, PATH_MAX);
  return std::string(result, (count > 0) ? count : 0);
#endif
}

} // namespace zod
