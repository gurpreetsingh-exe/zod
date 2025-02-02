#include "context.hh"

using namespace zod;

auto main(int argc, char** argv) -> int {
  if (argc < 2) {
    eprintln("no input file");
  }
  auto path = fs::path(argv[1]);
  if (not fs::exists(path)) {
    eprintln("\"{}\" does not exist", path.string());
  }
  if (fs::is_directory(path)) {
    eprintln("\"{}\" is a directory", path.string());
  }
  ZCtxt::create(fs::canonical(fs::absolute(argv[0])).parent_path());
  ZCtxt::get().run(std::move(path));
  ZCtxt::drop();
}
