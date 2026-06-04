#include "panel.hh"

#include "sodium/theme.hh"
#include "sodium/widgets/builder.hh"

namespace zod {

using namespace sodium;

struct PanelInner : CompoundWidget {
  PanelInner(Vector<SharedPtr<Widget>> widgets) : widgets(widgets) {
    auto idx = 0;
    auto tabs = create<HorizontalBox>().gap(2);
    auto theme = Theme::get();
    for (auto widget : widgets) {
      auto tab_header =
          *create<Box>()
               .background(idx == 0 ? theme.foreground : theme.background)
               .rounding(5.0f)
               .rounding_flags(RECT_ROUND_TOP)[create<Text>()
                                                   .padding({ 12, 12, 0, 0 })
                                                   .text(widget->name())]
               .on_mouse_down([this, idx](auto&) {
                 this->set_active(idx);
                 return EventResponse::handled();
               });
      tab_headers.push_back(tab_header);
      tabs + slot().fixed_width(120)[tab_header];
      idx++;
    }
    tabs + slot()[create<CompoundWidget>()];
    this->tabs = *tabs;
  }

  auto set_active(usize idx) -> void {
    ZASSERT(idx < widgets.size());
    tab_headers[active]->style().background = Theme::get().background;
    tab_headers[idx]->style().background = Theme::get().foreground;
    active = idx;
    invalidate_layout();
  }

  auto compute_desired_size(vec2 desired) -> vec2 override {
    return widgets[active]->compute_desired_size(desired);
  }

  auto arrange(const Rect& r) -> void override {
    return widgets[active]->arrange(r);
  }

  auto paint(PaintCx& cx) const -> void override { widgets[active]->paint(cx); }

  auto get_children() const -> WidgetChildren override {
    return { widgets[active] };
  }

  Vector<SharedPtr<Widget>> widgets;
  Vector<SharedPtr<Widget>> tab_headers;
  SharedPtr<Widget> tabs;
  usize active = 0;
};

Panel::Panel(Vector<SharedPtr<Widget>> widgets) {
  auto view = shared<PanelInner>(widgets);
  add_child(view->tabs, slot().fixed_height(25));
  add_child(view);
}

} // namespace zod
