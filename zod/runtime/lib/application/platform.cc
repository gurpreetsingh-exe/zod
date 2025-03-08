#ifdef PLATFORM_WINDOWS
#  include <windows.h>
#else
#  include <limits.h>
#  include <unistd.h>
#endif

#include "application/application.hh"
#include "application/platform.hh"

namespace zod {

auto open_dialog(DialogMode mode, SelectionMode smode) -> String {
  char buf[128] = {};
  const fs::path& default_path = Application::get().working_directory() / "";
#ifdef PLATFORM_WINDOWS
#  error "open_dialog not implemented for windows"
#else
  const auto smode_s = smode == SelectionMode::File ? "" : "--directory";
  auto mode_s = mode == DialogMode::Open ? "" : "--save";
  auto cmd = fmt::format(
      "zenity --file-selection --title=\"Select File\" {} {} --filename=\"{}\"",
      smode_s, mode_s, default_path.string());
  auto* f = popen(cmd.c_str(), "r");
  fgets(buf, 128, f);
  buf[strlen(buf) - 1] = '\0';
#endif
  return buf;
}

} // namespace zod
