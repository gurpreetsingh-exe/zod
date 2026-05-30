#include "brdf.glsl"
#include "tonemapping.glsl"

in vec3 P;
in vec3 N;
in vec2 uv;
flat in uint ID;

out vec4 O_color;

uint invalid_texture = 4294967295u;

uniform sampler2D u_mega_texture;

vec2 get_texture_coords(uint texture_index) {
  TextureInfo texture_info = tinfo[texture_index];
  vec2 mega_texture_size = vec2(textureSize(u_mega_texture, 0));
  return ((fract(uv) * texture_info.size) + texture_info.offset) /
         mega_texture_size;
}

vec3 fetch_color(uint texture_index, vec3 otherwise) {
  vec2 coords = get_texture_coords(texture_index);
  return texture_index != invalid_texture ? texture(u_mega_texture, coords).rgb
                                          : otherwise;
}

void main(void) {
  MeshInfo mesh_info = minfo[ID];
  vec3 V = normalize(ro.xyz - P);
  vec3 n = N * (gl_FrontFacing ? 1.0f : -1.0f);

  vec3 base_color =
      fetch_color(mesh_info.base_color_texture_index, vec3(0, 1, 0));
  // vec3 normal = fetch_color(mesh_info.normal_texture_index, n);
  vec3 r = fetch_color(mesh_info.roughness_texture_index, vec3(0.5f));

  float metallic = r.b;
  float roughness = r.g;
  vec3 color = get_color(P, V, base_color, n, metallic, roughness);
  // O_color = vec4(color, 1.0f);
  O_color = vec4(tonemap(color), 1.0f);
}
