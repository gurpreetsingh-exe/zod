#pragma once

#include "application/window.hh"

auto main(int, char**) -> int;

namespace zod {

class ILayer;

struct ApplicationCreateInfo {
  String name;
  Vector<String> cmd_args = {};
  fs::path working_directory = {};

  ApplicationCreateInfo(String /* name */, int /* argc */, char** /* argv */);
};

class Application {
public:
  Application(const ApplicationCreateInfo&);
  virtual ~Application();

public:
  static auto create(int /* argc */, char** /* argv */) -> Application*;
  static auto get() -> Application&;
  auto active_window() const -> Window&;
  auto on_event(Event&) -> void;
  auto set_working_directory(fs::path) -> void;
  auto working_directory() const -> const fs::path&;
  auto args() const -> const Vector<String>&;
  auto push_layer(ILayer*) -> void;

private:
  auto run() -> void;

protected:
  ApplicationCreateInfo m_info;
  UniquePtr<Window> m_window;
  bool m_running;

  Vector<ILayer*> m_layers;

  friend auto ::main(int argc, char** argv) -> int;
};

class ILayer {
public:
  virtual ~ILayer() = default;

private:
  virtual auto setup() -> void = 0;
  virtual auto update() -> void = 0;
  virtual auto on_event(Event&) -> void = 0;

  friend class Application;
};

} // namespace zod
