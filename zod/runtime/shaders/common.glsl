// OpenGL: 1, Vulkan: -1
#define CLIP_SPACE_UV_Y_DIR 1

vec2 cs_to_uv(vec2 cs) {
  return cs * vec2(0.5, CLIP_SPACE_UV_Y_DIR * 0.5) + vec2(0.5);
}

vec2 uv_to_cs(vec2 uv) {
  return uv - vec2(0.5) * vec2(2, CLIP_SPACE_UV_Y_DIR * 2);
}

// https://stackoverflow.com/a/32246825
// this is supposed to get the world position from the depth buffer
vec3 view_space_position_from_depth(float depth, vec2 coords) {
  float z = depth * 2.0 - 1.0;

  vec4 clip_space_pos = vec4(coords * 2.0 - 1.0, z, 1.0);
  vec4 view_space_pos = inv_projection * clip_space_pos;

  // Perspective division
  view_space_pos /= view_space_pos.w;
  return view_space_pos.xyz;
}

vec3 world_space_position_from_depth(float depth, vec2 coords) {
  vec3 view_space_pos = view_space_position_from_depth(depth, coords);
  vec4 world_pos = inv_view * vec4(view_space_pos, 1.0f);
  return world_pos.xyz;
}
