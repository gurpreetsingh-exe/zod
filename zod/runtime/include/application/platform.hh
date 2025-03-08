#pragma once

namespace zod {

enum class DialogMode {
  Save,
  Open,
};

enum class SelectionMode {
  File,
  Directory,
};

auto open_dialog(DialogMode = DialogMode::Open,
                 SelectionMode = SelectionMode::File) -> String;

} // namespace zod
