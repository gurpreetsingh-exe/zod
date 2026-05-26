// OpenGL: 1, Vulkan: -1
#define CLIP_SPACE_UV_Y_DIR 1

vec2 cs_to_uv(vec2 cs) {
  return cs * vec2(0.5, CLIP_SPACE_UV_Y_DIR * 0.5) + vec2(0.5);
}

vec2 uv_to_cs(vec2 uv) {
  return uv - vec2(0.5) * vec2(2, CLIP_SPACE_UV_Y_DIR * 2);
}
