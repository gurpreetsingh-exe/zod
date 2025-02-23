#pragma once

#ifdef _WIN32
#  ifdef _WIN64
#    define PLATFORM_WINDOWS
#  else
#    error "32-bit windows not supported"
#  endif
#elif __APPLE__
#  error "apple platform not supported"
#elif __ANDROID__
#  error "android platform not supported"
#elif __linux__
#  define PLATFORM_LINUX
#else
#  error "unknown platform"
#endif
