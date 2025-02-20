#include "./context.hh"

namespace zod {

VKContext::VKContext(void* glfw_window) { m_window = glfw_window; }

VKContext::~VKContext() {}

} // namespace zod
