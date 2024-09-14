add_library(uuid_v4 INTERFACE)
target_include_directories(uuid_v4 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(uuid_v4 INTERFACE -march=native)
