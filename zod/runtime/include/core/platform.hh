#pragma once

namespace zod {

auto open_dialog(const fs::path&) -> std::string;
auto get_exe_path() -> fs::path;

} // namespace zod
