#ifdef PLATFORM_WINDOWS
#  include <windows.h>
#else
#  include <fcntl.h>
#  include <limits.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#  include <sys/types.h>
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

auto memory_map(const fs::path& path) -> Mapping {
#ifdef PLATFORM_WINDOWS
  TODO();
#else
  auto fd = open(path.string().c_str(), O_RDONLY);
  if (fd == -1) {
    eprintln("cannot open file \"{}\"", path.string());
    return {};
  }

  struct stat sb;
  if (fstat(fd, &sb) == -1) {
    eprintln("fstat failed \"{}\", fd: {}", fd, path.string());
    return {};
  }

  void* addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (addr == MAP_FAILED) {
    eprintln("mmap failed \"{}\", fd: {}", fd, path.string());
    return {};
  }

  close(fd);
  return { addr, usize(sb.st_size) };
#endif
}

auto memory_unmap(Mapping mapping) -> void {
#ifdef PLATFORM_WINDOWS
  TODO();
#else
  if (munmap(mapping.page, mapping.size) == -1) {
    eprintln("munmap failed");
  }
#endif
}

} // namespace zod
