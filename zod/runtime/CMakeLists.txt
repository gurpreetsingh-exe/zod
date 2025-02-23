set(INC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_subdirectory(${THIRD_PARTY_DIR}/entt EnTT)
add_subdirectory(${THIRD_PARTY_DIR}/fmt fmt)
add_subdirectory(${THIRD_PARTY_DIR}/freetype freetype)
add_subdirectory(${THIRD_PARTY_DIR}/glfw glfw)
add_subdirectory(${THIRD_PARTY_DIR}/glm glm)
add_subdirectory(${THIRD_PARTY_DIR}/stbi stbi)
add_subdirectory(${THIRD_PARTY_DIR}/tinyobj tinyobj)
add_subdirectory(${THIRD_PARTY_DIR}/uuid_v4 uuid_v4)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/shaders shaderlib)

# precompiled header
add_library(zodpch INTERFACE)
target_precompile_headers(zodpch INTERFACE ${INC}/core/types.hh)
target_include_directories(zodpch INTERFACE ${INC})
target_link_libraries(zodpch INTERFACE fmt glm::glm)

# embed
add_executable(embed ${CMAKE_CURRENT_SOURCE_DIR}/lib/embed/main.cc)
target_link_libraries(embed zodpch)

# gpu
file(GLOB_RECURSE GPU_OPENGL_LIB ${LIB}/gpu/opengl/*.cc)
file(GLOB_RECURSE GPU_VULKAN_LIB ${LIB}/gpu/vulkan/*.cc)

set(GPU_LIB
  ${LIB}/gpu/context.cc
  ${LIB}/gpu/types.cc
  ${LIB}/gpu/platform.cc
  ${LIB}/gpu/shader.cc)

if (VULKAN_BACKEND)
  list(APPEND GPU_LIB ${GPU_VULKAN_LIB})
elseif (OPENGL_BACKEND)
  list(APPEND GPU_LIB ${GPU_OPENGL_LIB})
endif()

add_library(gpu ${GPU_LIB})
target_include_directories(gpu PUBLIC ${INC})
target_link_libraries(gpu zodpch shaderlib stbi)

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

file(GLOB_RECURSE GPU_LIB2 ${LIB}/gpu/*.cc)
file(GLOB_RECURSE RUNTIME_LIB ${LIB}/*.cc)
list(REMOVE_ITEM RUNTIME_LIB ${GPU_LIB2})

add_library(zrt ${RUNTIME_LIB})
if (VULKAN_BACKEND)
  target_compile_definitions(zrt PRIVATE VULKAN_BACKEND)
elseif (OPENGL_BACKEND)
  target_compile_definitions(zrt PRIVATE OPENGL_BACKEND)
endif()
target_include_directories(zrt PUBLIC ${INC})
target_link_libraries(zrt
  zodpch
  EnTT::EnTT
  freetype
  glfw
  gpu
  tinyobj
  uuid_v4)
