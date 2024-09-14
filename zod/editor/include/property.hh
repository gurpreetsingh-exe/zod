#pragma once

namespace zod {

enum {
  PROP_STRING,
  PROP_INT,
  PROP_FLOAT,
  PROP_VEC3,
};

enum {
  PROP_SUBTYPE_NONE = 0,
  PROP_SUBTYPE_FILEPATH,
};

struct Property {
  u8 type;
  bool needs_update;
  u8 subtype = PROP_SUBTYPE_NONE;
  const char* name;
  union {
    char* s;
    i32 i;
    f32 f;
    vec3 v3;
  };

  Property(const char* n, char* string)
      : type(PROP_STRING), name(n), s(string) {
    std::memset(s, 0, 64);
  }
  Property(const char* n, char* string, u8 subty)
      : type(PROP_STRING), name(n), s(string), subtype(subty) {
    std::memset(s, 0, 64);
  }
  Property(const char* n, i32 v) : type(PROP_INT), name(n), i(v) {}
  Property(const char* n, f32 v) : type(PROP_FLOAT), name(n), f(v) {}
  Property(const char* n, vec3 v) : type(PROP_VEC3), name(n), v3(v) {}

  Property(Property&&);
  auto draw() -> bool;

  ~Property();
};

} // namespace zod
