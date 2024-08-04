#pragma once

namespace zod {

/// Command line options
struct Config {
  /// Input filename
  fs::path input;
  /// Output filename
  Option<fs::path> output;
};

} // namespace zod
