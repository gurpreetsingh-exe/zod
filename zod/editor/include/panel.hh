#pragma once

#include "sodium/widgets/box_container.hh"

namespace zod {

struct Panel : sodium::VerticalBox {
  Panel(Vector<SharedPtr<Widget>>);
};

} // namespace zod
