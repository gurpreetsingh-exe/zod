add_library(stbi stb_image.cc)
target_include_directories(stbi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
