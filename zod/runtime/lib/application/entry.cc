#include "application/application.hh"

auto main(int argc, char** argv) -> int {
  auto app = zod::Application::create(argc, argv);
  app->run();
  delete app;
}
