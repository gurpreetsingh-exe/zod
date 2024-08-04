#include "session/config.hh"

namespace zod {

constexpr std::string_view usage_string = R"(usage: {} [options] input...

Options:
    -h, --help            Display this message
)";

[[noreturn]]
auto usage(const std::string& arg0) {
  fmt::println(usage_string, arg0);
  std::exit(1);
}

auto config_from_args(i32 argc, char** argv) -> Config {
  char* arg0 = argv[0];
  if (argc < 2) {
    usage(arg0);
  }
  ++argv;
  --argc;

  auto config = Config {};

  for (usize i = 0; i < static_cast<usize>(argc); ++i) {
    std::string_view arg = argv[i];
    if (arg[0] == '-') {
      switch (arg[1]) {
        case '-': {
          if (arg == "--help") {
            usage(arg0);
          } else {
            fmt::println(stderr, "Unknown option `{}`", arg.data());
          }
          break;
        }
        case 'h':
          usage(arg0);
        default:
          fmt::println(stderr, "Unknown option `{}`", arg.data());
      }
    } else {
      if (arg.empty())
        continue;
      if (config.input.empty()) {
        config.input = arg;
      } else {
        fmt::println(stderr, "multiple input files provided");
        usage(arg0);
      }
    }
  }

  if (config.input.empty()) {
    fmt::println("no input file provided");
    usage(arg0);
  }

  return config;
}

} // namespace zod
