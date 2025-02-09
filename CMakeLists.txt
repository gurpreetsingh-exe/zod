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
set(SODIUM_DIR ${CMAKE_SOURCE_DIR}/zod/sodium)
set(ZOD_SHADERS_DIR ${ZOD_GPU_DIR}/shaders)
set(ZOD_COMPILER_DRIVER ${ZOD_COMPILER_DIR}/lib/driver/zodc.cc)
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third-party)
set(IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)

add_compile_options(-msse3 -ggdb3)
# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

add_subdirectory(${THIRD_PARTY_DIR}/fmt fmt)
add_subdirectory(${THIRD_PARTY_DIR}/glm glm)
add_subdirectory(${THIRD_PARTY_DIR}/glfw glfw)
add_subdirectory(${THIRD_PARTY_DIR}/tinyobj tinyobj)
add_subdirectory(${THIRD_PARTY_DIR}/stbi stbi)
add_subdirectory(${THIRD_PARTY_DIR}/freetype freetype)
add_subdirectory(${THIRD_PARTY_DIR}/uuid_v4 uuid_v4)
add_subdirectory(${THIRD_PARTY_DIR}/entt EnTT)
add_subdirectory(${THIRD_PARTY_DIR}/json nlohmann_json)

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
target_link_libraries(zodcore zodpch glm::glm tinyobj EnTT::EnTT)

##########################################################################
# libgpu
##########################################################################
file(GLOB_RECURSE ZOD_GPU_OPENGL_SRC ${ZOD_GPU_DIR}/lib/opengl/*.cc)
file(GLOB_RECURSE ZOD_GPU_VULKAN_SRC ${ZOD_GPU_DIR}/lib/vulkan/*.cc)
file(GLOB_RECURSE ZOD_GPU_SHADER_LIB ${ZOD_GPU_DIR}/lib/shaders/*.cc)

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

set(ZOD_GPU_SRC
  ${ZOD_GPU_DIR}/lib/context.cc
  ${ZOD_GPU_DIR}/lib/types.cc
  ${ZOD_GPU_DIR}/lib/platform.cc
  ${ZOD_GPU_DIR}/lib/shader.cc)

list(APPEND ZOD_GPU_SRC ${ZOD_GPU_SHADER_LIB})

if (VULKAN_BACKEND)
  list(APPEND ZOD_GPU_SRC ${ZOD_GPU_VULKAN_SRC})
elseif (OPENGL_BACKEND)
  list(APPEND ZOD_GPU_SRC ${ZOD_GPU_OPENGL_SRC})
endif()

add_library(gpu ${ZOD_GPU_SRC})

if (VULKAN_BACKEND)
  target_compile_definitions(gpu PRIVATE VULKAN_BACKEND)
  find_package(Vulkan REQUIRED)
  add_subdirectory(${THIRD_PARTY_DIR}/VulkanMemoryAllocator VulkanMemoryAllocator)
  set(ENABLE_HLSL OFF)
  set(ENABLE_OPT OFF)
  set(ALLOW_EXTERNAL_SPIRV_TOOLS OFF)
  add_subdirectory(${THIRD_PARTY_DIR}/glslang glslang)
  add_subdirectory(${THIRD_PARTY_DIR}/vk-bootstrap vk-bootstrap)
  target_link_libraries(gpu
    Vulkan::Vulkan
    vk-bootstrap
    glslang::glslang
    glslang::glslang-default-resource-limits
    GPUOpen::VulkanMemoryAllocator)
elseif (OPENGL_BACKEND)
  target_compile_definitions(gpu PRIVATE OPENGL_BACKEND)
  find_package(OpenGL REQUIRED)
  add_subdirectory(${THIRD_PARTY_DIR}/glad glad)
  target_link_libraries(gpu OpenGL::GL glad)
endif()

target_include_directories(gpu PUBLIC ${ZOD_CORE_DIR}/include)
target_include_directories(gpu PUBLIC ${ZOD_GPU_DIR}/include)
add_subdirectory(${ZOD_GPU_DIR} shaderlib)
target_link_libraries(gpu shaderlib zodpch stbi)
target_compile_options(gpu PRIVATE -fsanitize=address)
target_link_options(gpu PUBLIC -fsanitize=address)

##########################################################################
# sodium ui
##########################################################################
file(GLOB_RECURSE SODIUM_SRC ${SODIUM_DIR}/lib/*.cc)
add_library(sodium ${SODIUM_SRC})
if (VULKAN_BACKEND)
  target_compile_definitions(sodium PRIVATE VULKAN_BACKEND)
elseif (OPENGL_BACKEND)
  target_compile_definitions(sodium PRIVATE OPENGL_BACKEND)
endif()
target_include_directories(sodium PUBLIC ${ZOD_CORE_DIR}/include)
target_include_directories(sodium PUBLIC ${SODIUM_DIR}/include)
target_link_libraries(sodium zodpch zodcore gpu glm::glm glfw imgui nlohmann_json::nlohmann_json freetype pthread ${CMAKE_DL_LIBS})

##########################################################################
# zod editor
##########################################################################
file(GLOB_RECURSE ZOD_EDITOR_SRC ${ZOD_EDITOR_DIR}/lib/*.cc)
add_executable(zod ${ZOD_EDITOR_SRC})
if (VULKAN_BACKEND)
  target_compile_definitions(zod PRIVATE VULKAN_BACKEND)
elseif (OPENGL_BACKEND)
  target_compile_definitions(zod PRIVATE OPENGL_BACKEND)
endif()
target_compile_definitions(zod PUBLIC GLFW_INCLUDE_NONE=1)
target_include_directories(zod PUBLIC ${ZOD_CORE_DIR}/include)
target_include_directories(zod PUBLIC ${ZOD_EDITOR_DIR}/include)
target_link_libraries(zod zodpch zodcore gpu sodium glm::glm uuid_v4)

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
