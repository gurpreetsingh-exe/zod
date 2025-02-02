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

} // namespace zod
