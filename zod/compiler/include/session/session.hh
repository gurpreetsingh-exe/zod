#pragma once

#include "config.hh"
#include "parse.hh"

namespace zod {

/// Represents the data associated with a compilation session
struct Session {
  /// Command line options
  Config config;
  /// Parsing session
  ParseSess parseSess;

  Session(Config cfg) : config(cfg) {}
  auto run_compiler() -> void;
};

} // namespace zod
