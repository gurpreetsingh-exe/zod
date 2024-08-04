#pragma once

#define WRAP_UNWRAP(Opaque, Type)                                              \
  static inline auto wrap(Type* ctx)->Opaque* {                                \
    return reinterpret_cast<Opaque*>(ctx);                                     \
  }                                                                            \
  static inline auto unwrap(Opaque* ctx)->Type* {                              \
    return reinterpret_cast<Type*>(ctx);                                       \
  }                                                                            \
  static inline auto unwrap(const Opaque* ctx)->const Type* {                  \
    return reinterpret_cast<const Type*>(ctx);                                 \
  }
