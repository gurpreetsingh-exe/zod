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

struct DialogOptions {
  DialogMode dialog_mode = DialogMode::Open;
  SelectionMode selection_mode = SelectionMode::File;
  String filter = "*";
};

auto open_dialog(DialogOptions = DialogOptions()) -> String;

} // namespace zod
