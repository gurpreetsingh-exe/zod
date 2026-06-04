#include "debug.hh"
#include "editor.hh"
#include "gpu/timer.hh"
#include "sodium/theme.hh"
#include "sodium/widgets/builder.hh"
#include "sodium/widgets/list_view.hh"

namespace zod {

using namespace sodium;

static auto text_widgets(const SharedPtr<Widget>& root)
    -> Vector<SharedPtr<Text>> {
  auto widgets = Vector<SharedPtr<Text>>();
  if (not root) {
    return widgets;
  }
  if (auto text = std::dynamic_pointer_cast<Text>(root)) {
    widgets.push_back(text);
  }
  for (auto child : root->get_children()) {
    auto child_widgets = text_widgets(child);
    widgets.insert(widgets.end(), child_widgets.begin(), child_widgets.end());
  }
  return widgets;
}

Debug::Debug()
    : CompoundWidget(),
      m_debug_timings(shared<ListView<std::pair<String, f32>>>()),
      m_hover_paths(shared<ListView<String>>()) {
  // set_hover_path_visible(false);

  auto theme = Theme::get();
  set_child(*(create<VerticalBox>() +
              slot().auto_height()[create<Box>().padding(4).background(
                  theme.background)[m_debug_timings]] +
              slot()[m_hover_paths]));

  m_debug_timings->set_on_generate_row([](auto pair, usize i) {
    auto theme = Theme::get();
    auto [name, time] = pair;
    auto text = [](String name) {
      return *create<Text>(name).padding({ 8, 8, 0, 0 });
    };
    return *create<Box>().background(
        theme.background)[create<HorizontalBox>().gap(2) + slot()[text(name)] +
                          slot().fixed_width(
                              2)[create<Box>().background(theme.foreground)] +
                          slot()[text(fmt::format("{}", time))]];
  });
  m_debug_timings->set_on_update_row(
      [](const SharedPtr<Widget>& row, auto pair, usize) {
        auto texts = text_widgets(row);
        if (texts.size() >= 2) {
          auto [name, time] = pair;
          texts[0]->set_text(name);
          texts[1]->set_text(fmt::format("{}", time));
        }
      });
  m_debug_timings->set_items_source([] {
    auto item = Vector<std::pair<String, f32>>();
    for (auto& i : GPUTimer::get().timings()) {
      item.push_back({ "GPU " + i.first, i.second });
    }
    return item;
  });

  m_hover_paths->set_on_generate_row([](auto& name, usize i) {
    return *create<Text>(name).padding({ 8, 8, 0, 0 });
  });
  m_hover_paths->set_on_update_row(
      [](const SharedPtr<Widget>& row, const String& name, usize) {
        if (auto text = std::dynamic_pointer_cast<Text>(row)) {
          text->set_text(name);
        }
      });
  m_hover_paths->set_items_source([] {
    auto root = Editor::get().get_root_widget();
    return root->debug_hover_path();
  });
}

auto Debug::request_refresh() -> void {
  m_debug_timings->request_refresh();
  m_hover_paths->request_refresh();
}

} // namespace zod
