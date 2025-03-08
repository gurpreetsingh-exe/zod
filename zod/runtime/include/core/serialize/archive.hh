#pragma once

namespace zod {

class Archive {
public:
  Archive() = default;
  ~Archive() = default;

public:
  auto copy(u8*, usize) -> void;
  auto save(fs::path, const String& /* extension */ = "") const -> void;

private:
  Vector<u8> m_buf = {};
  usize m_idx = 0;
};

} // namespace zod
