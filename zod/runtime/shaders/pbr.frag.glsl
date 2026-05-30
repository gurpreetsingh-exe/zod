#include "brdf.glsl"
#include "tonemapping.glsl"

#include "common.glsl"

in vec2 uv;

uniform sampler2D u_albedo;
uniform sampler2D u_normal;
uniform sampler2D u_roughness;
uniform sampler2D u_depth;

layout(location = 0) out vec4 O_color;

vec3 get_normal(vec2 coords) {
  return normalize(texture(u_normal, coords).rgb * 2.0f - 1.0f);
}

void main() {
  vec3 P = world_space_position_from_depth(texture(u_depth, uv).x, uv);
  vec3 V = normalize(ro.xyz - P);

  vec3 base_color = texture(u_albedo, uv).rgb;
  vec3 N = get_normal(uv);
  vec3 r = texture(u_roughness, uv).rgb;

  float metallic = r.x;
  float roughness = r.y;

  vec3 color = get_color(P, V, base_color, N, metallic, roughness);
  O_color = vec4(tonemap(color), 1.0f);
}
