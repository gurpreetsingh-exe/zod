file(GLOB_RECURSE SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.glsl)

set(SHADER_GENERATED)
set(SHADER_INC)
make_directory(${PROJECT_BINARY_DIR}/shaders_generated/include)

string(APPEND SHADER_INC "#pragma once\n")

foreach(SHADER ${SHADERS})
  get_filename_component(OUT_FILE ${SHADER} NAME_WLE)
  set(OUT ${PROJECT_BINARY_DIR}/shaders_generated/${OUT_FILE}.cc)
  add_custom_command(
    OUTPUT ${OUT}
    DEPENDS embed
    COMMAND embed ${SHADER} > ${OUT})

  list(APPEND SHADER_GENERATED ${OUT})
  string(REPLACE "." "_" VAR_NAME ${OUT_FILE})
  string(APPEND SHADER_INC "extern const char* g_${VAR_NAME}_src\;\n")
endforeach(SHADER)

file(WRITE ${PROJECT_BINARY_DIR}/shaders_generated/include/shaders_generated.hh ${SHADER_INC})

add_library(shaderlib ${SHADER_GENERATED})
target_include_directories(shaderlib PUBLIC ${PROJECT_BINARY_DIR}/shaders_generated/include)
