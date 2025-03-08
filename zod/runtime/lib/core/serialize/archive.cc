#include "core/serialize/archive.hh"

namespace zod {

auto Archive::copy(u8* buf, usize size) -> void {
  // m_buf.resize(size + m_idx);
  // std::memcpy(&m_buf[m_idx], buf, size);
  m_buf.insert(m_buf.end(), buf, buf + size);
  m_idx += size;
}

auto Archive::save(fs::path path, const String& extension) const -> void {
  if (not extension.empty()) {
    path.replace_extension(extension);
  }
  auto* f = std::fopen(path.string().c_str(), "wb");
  std::fwrite(m_buf.data(), 1, m_buf.size(), f);
  std::fclose(f);
}

} // namespace zod
