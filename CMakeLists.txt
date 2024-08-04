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

set(ZOD_CORE_DIR ${CMAKE_SOURCE_DIR}/zod/core)
set(ZOD_EDITOR_DIR ${CMAKE_SOURCE_DIR}/zod/editor)
set(ZOD_COMPILER_DIR ${CMAKE_SOURCE_DIR}/zod/compiler)
set(ZOD_GPU_DIR ${CMAKE_SOURCE_DIR}/zod/gpu)
set(ZOD_SHADERS_DIR ${ZOD_GPU_DIR}/shaders)
set(ZOD_COMPILER_DRIVER ${ZOD_COMPILER_DIR}/lib/driver/zodc.cc)
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third-party)
set(IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)

# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

add_subdirectory(${THIRD_PARTY_DIR}/fmt fmt)
add_subdirectory(${THIRD_PARTY_DIR}/glm glm)
add_subdirectory(${THIRD_PARTY_DIR}/glfw glfw)
add_subdirectory(${THIRD_PARTY_DIR}/tinyobj tinyobj)
add_subdirectory(${THIRD_PARTY_DIR}/glad glad)
find_package(OpenGL REQUIRED)

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
# precompiled header
##########################################################################
add_library(zodpch INTERFACE)
target_precompile_headers(zodpch INTERFACE ${ZOD_CORE_DIR}/include/base/base.hh)
target_link_libraries(zodpch INTERFACE fmt)

##########################################################################
# embed
##########################################################################
add_executable(embed ${CMAKE_SOURCE_DIR}/zod/embed/lib/main.cc)
target_link_libraries(embed zodpch)

##########################################################################
# libzodcore
##########################################################################
file(GLOB_RECURSE ZOD_CORE_SRC ${ZOD_CORE_DIR}/lib/*.cc)
add_library(zodcore ${ZOD_CORE_SRC})
target_include_directories(zodcore PUBLIC ${ZOD_CORE_DIR}/include)
target_link_libraries(zodcore zodpch glm::glm tinyobj)

##########################################################################
# libgpu
##########################################################################
file(GLOB_RECURSE ZOD_GPU_SRC ${ZOD_GPU_DIR}/lib/*.cc)
add_library(gpu ${ZOD_GPU_SRC})
target_include_directories(gpu PUBLIC ${ZOD_CORE_DIR}/include)
target_include_directories(gpu PUBLIC ${ZOD_GPU_DIR}/include)
target_link_libraries(gpu zodpch OpenGL::GL glad)

##########################################################################
# zod editor
##########################################################################
file(GLOB_RECURSE ZOD_EDITOR_SRC ${ZOD_EDITOR_DIR}/lib/*.cc)
add_executable(zod ${ZOD_EDITOR_SRC})
target_compile_definitions(zod PUBLIC GLFW_INCLUDE_NONE=1)
target_include_directories(zod PUBLIC ${ZOD_CORE_DIR}/include)
target_include_directories(zod PUBLIC ${ZOD_EDITOR_DIR}/include)
target_link_libraries(zod zodpch zodcore gpu glm::glm glfw $<$<CONFIG:DEBUG>:imgui> pthread ${CMAKE_DL_LIBS})

##########################################################################
# libzc
##########################################################################
file(GLOB_RECURSE ZOD_COMPILER_SRC ${ZOD_COMPILER_DIR}/lib/*.cc)
list(REMOVE_ITEM ZOD_COMPILER_SRC ${ZOD_COMPILER_DRIVER})
add_library(zc ${ZOD_COMPILER_SRC})
target_include_directories(zc PUBLIC ${ZOD_CORE_DIR}/include)
target_include_directories(zc PUBLIC ${ZOD_COMPILER_DIR}/include)
target_link_libraries(zc zodpch)

target_link_libraries(zodcore zc)

##########################################################################
# zodc compiler
##########################################################################
add_executable(zodc ${ZOD_COMPILER_DRIVER})
target_include_directories(zodc PUBLIC ${ZOD_CORE_DIR}/include)
target_include_directories(zodc PUBLIC ${ZOD_COMPILER_DIR}/include)
target_link_libraries(zodc zc zodcore)
