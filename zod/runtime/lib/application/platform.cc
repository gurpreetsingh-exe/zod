#ifdef PLATFORM_WINDOWS
#  include <windows.h>
#else
#  include <limits.h>
#  include <unistd.h>
#endif

namespace zod {

auto open_dialog(const fs::path& default_path) -> String {
  char buf[128] = {};
#ifdef PLATFORM_WINDOWS
#  error "open_dialog not implemented for windows"
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

} // namespace zod
