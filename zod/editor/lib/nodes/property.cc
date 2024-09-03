#include <imgui.h>

#include "property.hh"

namespace zod {

Property::Property(Property&& prop) : name(prop.name), type(prop.type) {
  switch (prop.type) {
    case PROP_STRING: {
      s = prop.s;
      prop.s = nullptr;
    } break;
    case PROP_INT: {
      i = prop.i;
    } break;
    case PROP_FLOAT: {
      f = prop.f;
    } break;
  }
}

auto Property::draw() -> void {
  switch (type) {
    case PROP_STRING: {
      needs_update = ImGui::InputText(name, s, 64);
    } break;
    case PROP_INT: {
      needs_update = ImGui::DragInt(name, &i);
    } break;
    case PROP_FLOAT: {
      needs_update = ImGui::DragFloat(name, &f);
    } break;
  }
}

Property::~Property() {
  switch (type) {
    case PROP_STRING: {
      delete[] s;
    } break;
  }
}

} // namespace zod
