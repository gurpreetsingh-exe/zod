#include <cxxabi.h>

namespace zod {

auto demangle(const String& name) -> String {
  int status = -4;
  auto res = UniquePtr<char, void (*)(void*)> {
    abi::__cxa_demangle(name.c_str(), NULL, NULL, &status), std::free
  };

  return (status == 0) ? res.get() : name;
}

} // namespace zod
