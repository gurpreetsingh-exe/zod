#include "properties.hh"
#include "context.hh"

namespace zod {

Properties::Properties()
    : SPanel("Properties", unique<OrthographicCamera>(64.0f, 64.0f)) {}

auto Properties::update() -> void {}

} // namespace zod
