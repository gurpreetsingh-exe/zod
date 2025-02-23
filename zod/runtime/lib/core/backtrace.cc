#include <execinfo.h>

#include "core/backtrace.hh"

namespace zod {

constexpr bool backtrace_addr2line = true;
constexpr StringView backtrace_addr2line_options = "-p -C -f";
constexpr bool backtrace_llvm_symbolizer = false;
constexpr StringView backtrace_llvm_symbolizer_options =
    "-s -p -C -i --color --output-style=GNU";

auto print_backtrace() -> void {
  constexpr usize size = 128;
  void* trace[size];
  int n = backtrace(trace, size);
  char** strings = backtrace_symbols(&trace[0], size);
  int skip_value = 1;

  if constexpr (backtrace_addr2line) {
    for (int i = skip_value; i < n; ++i) {
      trace[i] = (void*)((std::uintptr_t)trace[i] - 1);
    }

    std::span<void*> trace_view { &trace[skip_value], &trace[n] };

    String command = fmt::format(
        "addr2line {} -e {} {}", backtrace_addr2line_options,
        fs::canonical("/proc/self/exe").native(), fmt::join(trace_view, " "));
    std::system(command.data());
  }
}

} // namespace zod
