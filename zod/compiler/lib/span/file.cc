#include "span/file.hh"

namespace zod {

auto read_file(fs::path path) -> std::string {
  FILE* f = std::fopen(path.c_str(), "r");

  if (not f) {
    eprintln("cannot open \"{}\"\n  {}", path.c_str(), std::strerror(errno));
  }

  if (std::fseek(f, 0L, SEEK_END)) {
    eprintln("{}", std::strerror(errno));
  }

  auto file_size = std::ftell(f);
  if (file_size < 0) {
    eprintln("cannot get file size\n  {}", std::strerror(errno));
  }

  std::rewind(f);

  auto size = usize(file_size);
  auto content = std::string();
  content.resize(size);
  auto read_bytes = std::fread(content.data(), 1, size, f);

  if (read_bytes != size) {
    int e = std::feof(f);
    if (e) {
      eprintln("unexpected end of file");
    } else {
      e = std::ferror(f);
      eprintln("cannot read file\n  {}", std::strerror(e));
    }
  }

  std::fclose(f);

  return content;
}

} // namespace zod
