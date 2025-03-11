cmake_minimum_required(VERSION 3.20.0)
project(zod VERSION 0.1)

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
  message("debug mode")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

set(ZOD_RUNTIME_DIR ${CMAKE_SOURCE_DIR}/zod/runtime)
set(ZOD_EDITOR_DIR ${CMAKE_SOURCE_DIR}/zod/editor)
set(SODIUM_DIR ${CMAKE_SOURCE_DIR}/zod/sodium)
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third-party)
set(IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)

if (GPU_BACKEND STREQUAL "OpenGL")
  message(STATUS "GPU Backend: OpenGL")
  set(OPENGL_BACKEND ON)
elseif (GPU_BACKEND STREQUAL "Vulkan")
  message(STATUS "GPU Backend: Vulkan")
  set(VULKAN_BACKEND ON)
else()
  message(STATUS "No GPU Backend: Defaulting to OpenGL")
  set(OPENGL_BACKEND ON)
endif()

add_compile_options(-msse3 -ggdb3)

add_subdirectory(${ZOD_RUNTIME_DIR} zrt)
add_subdirectory(${THIRD_PARTY_DIR}/json nlohmann_json)
add_subdirectory(${THIRD_PARTY_DIR}/tinyobj tinyobj)
add_subdirectory(${THIRD_PARTY_DIR}/tinygltf tinygltf)

add_library(imgui
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC "${IMGUI_DIR}")
target_include_directories(imgui PUBLIC "${IMGUI_DIR}/backends")

##########################################################################
# sodium ui
##########################################################################
file(GLOB_RECURSE SODIUM_SRC ${SODIUM_DIR}/lib/*.cc)
add_library(sodium ${SODIUM_SRC})
target_include_directories(sodium PUBLIC ${ZOD_RUNTIME_DIR}/include)
target_include_directories(sodium PUBLIC ${SODIUM_DIR}/include)
target_link_libraries(sodium
  zodpch
  zrt
  nlohmann_json::nlohmann_json
  imgui
  pthread
  ${CMAKE_DL_LIBS})

##########################################################################
# zod editor
##########################################################################
file(GLOB_RECURSE ZOD_EDITOR_SRC ${ZOD_EDITOR_DIR}/lib/*.cc)
add_executable(zod ${ZOD_EDITOR_SRC})
target_compile_definitions(zod PUBLIC GLFW_INCLUDE_NONE=1)
target_include_directories(zod PUBLIC ${ZOD_RUNTIME_DIR}/include)
target_include_directories(zod PUBLIC ${ZOD_EDITOR_DIR}/include)
target_link_libraries(zod zodpch zrt sodium tinyobj tinygltf)
