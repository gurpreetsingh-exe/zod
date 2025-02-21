#include "editor.hh"

namespace zod {

class ZodEditor : public Application {
public:
  ZodEditor(const ApplicationCreateInfo& info) : Application(info) {
    push_layer(new Editor());
  }
};

auto Application::create(int argc, char** argv) -> Application* {
  return new ZodEditor(ApplicationCreateInfo("Zod", argc, argv));
}

} // namespace zod
