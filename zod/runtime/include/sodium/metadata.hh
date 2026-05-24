#pragma once

#include "event_handler.hh"

namespace zod::sodium {

class IWidgetMetaData {
public:
  virtual ~IWidgetMetaData() = default;
  virtual auto type() const -> const std::type_info& = 0;

  template <class MetaDataT>
  auto is_of_type() const -> bool {
    return type() == typeid(MetaDataT);
  }
};

template <class MetaDataT>
class WidgetMetaData : public IWidgetMetaData {
public:
  auto type() const -> const std::type_info& override {
    return typeid(MetaDataT);
  }
};

struct WidgetMouseEventsMetaData
    : public WidgetMetaData<WidgetMouseEventsMetaData> {
  EventHandler mouse_down = {};
  EventHandler mouse_up = {};
  EventHandler mouse_move = {};
  EventHandler mouse_enter = {};
  EventHandler mouse_leave = {};
  EventHandler drag_detected = {};
};

} // namespace zod::sodium
