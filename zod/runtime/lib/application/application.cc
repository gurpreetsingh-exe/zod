#include "application/application.hh"
#include "engine/font.hh"
#include "engine/runtime.hh"

namespace zod {

ApplicationCreateInfo::ApplicationCreateInfo(String name_, int argc,
                                             char** argv)
    : name(name_) {
  for (int i = 0; i < argc; ++i) { cmd_args.push_back(String(argv[i])); }
}

static Application* g_instance = nullptr;

Application::Application(const ApplicationCreateInfo& info)
    : m_info(info), m_window(unique<Window>(info.name)), m_running(true) {
  ZASSERT(not g_instance);
  g_instance = this;

  if (m_info.working_directory.empty()) {
    set_working_directory(fs::current_path());
  }

  m_window->set_event_callback(std::bind(&Application::on_event, this, ph::_1));
  Runtime::init();
  init_font("../third-party/imgui/misc/fonts/DroidSans.ttf");
}

Application::~Application() { Runtime::destroy(); }

auto Application::get() -> Application& {
  ZASSERT(g_instance != nullptr);
  return *g_instance;
}

auto Application::active_window() const -> Window& {
  ZASSERT(m_window.get());
  return *m_window;
}

auto Application::working_directory() const -> const fs::path& {
  return m_info.working_directory;
}

auto Application::set_working_directory(fs::path dir) -> void {
  m_info.working_directory = std::move(dir);
}

auto Application::args() const -> const Vector<String>& {
  return m_info.cmd_args;
}

auto Application::push_layer(ILayer* layer) -> void {
  layer->setup();
  m_layers.push_back(layer);
}

auto Application::run() -> void {
  while (m_running) {
    for (auto* layer : m_layers) { layer->update(); }
    m_window->update();
  }
}

auto Application::on_event(Event& event) -> void {
  switch (event.kind) {
    case Event::WindowClose: {
      m_running = event.hanging = false;
      return;
    };
  }

  for (auto i = int(m_layers.size()) - 1; i >= 0; --i) {
    if (not event.hanging) {
      break;
    }

    m_layers[i]->on_event(event);
  }
}

} // namespace zod
