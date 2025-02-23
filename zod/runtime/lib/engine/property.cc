#include "engine/property.hh"

namespace zod {

Property::Property(Property&& prop)
    : name(prop.name), type(prop.type), subtype(prop.subtype) {
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
    case PROP_VEC3: {
      v3 = prop.v3;
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
