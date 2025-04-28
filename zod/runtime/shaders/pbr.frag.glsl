#include "brdf.glsl"
#include "tonemapping.glsl"

in vec2 uv;

uniform sampler2D u_albedo;
uniform sampler2D u_normal;
uniform sampler2D u_depth;

layout(location = 0) out vec4 O_color;

// https://stackoverflow.com/a/32246825
// this is supposed to get the world position from the depth buffer
vec3 world_pos_from_depth(float depth) {
  float z = depth * 2.0 - 1.0;

  vec4 clip_space_pos = vec4(uv * 2.0 - 1.0, z, 1.0);
  vec4 view_space_pos = inv_projection * clip_space_pos;

  // Perspective division
  view_space_pos /= view_space_pos.w;

  vec4 world_pos = inv_view * view_space_pos;

  return world_pos.xyz;
}

void main() {
  vec3 P = world_pos_from_depth(texture(u_depth, uv).x);
  vec3 v = normalize(ro.xyz - P);

  vec3 base_color = texture(u_albedo, uv).rgb;
  vec3 normal = texture(u_normal, uv).rgb;
  vec3 n = normal;

  float metallic = r.b;
  float roughness = r.g;

  vec3 color = get_color(P, v, base_color, n, metallic, roughness);
  O_color = vec4(tonemap(color), 1.0f);
}
