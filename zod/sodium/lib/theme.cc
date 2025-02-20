#include <fstream>
#include <nlohmann/json.hpp>

#include "core/config.hh"
#include "theme.hh"

namespace zod {

using json = nlohmann::json;

Theme g_theme;

#define ARRAY_TO_VEC4(jsonprop, prop)                                          \
  {                                                                            \
    float v[4] = {};                                                           \
    j.at(jsonprop).get_to(v);                                                  \
    prop = vec4(v[0], v[1], v[2], v[3]);                                       \
  }

static auto from_json(const json& j, Theme& theme) -> void {
  ARRAY_TO_VEC4("background", theme.background);
  ARRAY_TO_VEC4("highlight", theme.highlight);
  ARRAY_TO_VEC4("primary", theme.primary);
}

auto Theme::load() -> void {
  auto config = find_config_dir() / "editor.json";
  auto ss = std::ifstream(config);
  auto j = json::parse(ss);
  g_theme = j["theme"].template get<Theme>();
}

auto Theme::get() -> Theme& { return g_theme; }

} // namespace zod
