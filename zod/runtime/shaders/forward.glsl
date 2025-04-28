#include "brdf.glsl"
#include "tonemapping.glsl"

in vec3 P;
in vec3 N;
in vec2 uv;
flat in uint ID;

out vec4 O_color;

uniform sampler2D u_mega_texture;

vec2 get_texture_coords(uint texture_index) {
  TextureInfo texture_info = tinfo[texture_index];
  vec2 mega_texture_size = vec2(textureSize(u_mega_texture, 0));
  return ((fract(uv) * texture_info.size) + texture_info.offset) /
                mega_texture_size;
}

void main(void) {
  MeshInfo mesh_info = minfo[ID];
  vec3 v = normalize(ro.xyz - P);
  vec3 n = N;

  vec3 base_color = texture(u_mega_texture, get_texture_coords(mesh_info.base_color_texture_index)).rgb;
  vec3 normal = texture(u_mega_texture, get_texture_coords(mesh_info.normal_texture_index)).rgb;
  vec3 r = texture(u_mega_texture, get_texture_coords(mesh_info.roughness_texture_index)).rgb;

  float metallic = r.b;
  float roughness = r.g;

  vec3 color = get_color(P, v, base_color, n, metallic, roughness);
  O_color = vec4(tonemap(color), 1.0f);
}
