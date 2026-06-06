#include "application/application.hh"
#include "core/platform/platform.hh"

auto main(int argc, char** argv) -> int {
  zod::install_signal_handlers();

  auto app = zod::Application::create(argc, argv);
  app->run();
  delete app;
}
