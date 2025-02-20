#pragma once

#include "backend.hh"

namespace zod {

#define GPU_TIME(name, ...) GPUTimer::get().with_scope(name, [&] __VA_ARGS__);

class GPUTimer {
public:
  GPUTimer() = default;

public:
  static auto get() -> GPUTimer&;

  template <typename Callback>
  auto with_scope(const std::string& name, Callback cb) -> void {
    if (not m_queries.contains(name)) {
      m_queries[name] = GPUBackend::get().create_query();
    }
    auto query = m_queries[name];
    query->begin();
    cb();
    query->end();
    m_times[name] = query->get_time();
  }

  auto timings() const -> const std::unordered_map<std::string, f32>& {
    return m_times;
  }

private:
  std::unordered_map<std::string, Shared<GPUQuery>> m_queries;
  std::unordered_map<std::string, f32> m_times;
};

} // namespace zod
