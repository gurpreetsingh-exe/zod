#pragma once

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#define VK_CHECK(x)                                                            \
  do {                                                                         \
    VkResult err = x;                                                          \
    if (err) {                                                                 \
      fmt::println("Detected Vulkan error: {}", string_VkResult(err));         \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)
