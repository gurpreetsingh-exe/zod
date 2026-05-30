#include "buffers.glsl"

in vec3 N;
in vec2 uv;
flat in uint ID;

layout(location = 0) out vec4 O_color;
layout(location = 1) out vec4 O_normal;
layout(location = 2) out vec4 O_roughness;

uniform sampler2D u_mega_texture;

uint invalid_texture = 4294967295u;

vec2 get_texture_coords(uint texture_index) {
  TextureInfo texture_info = tinfo[texture_index];
  vec2 mega_texture_size = vec2(textureSize(u_mega_texture, 0));
  return ((fract(uv) * texture_info.size) + texture_info.offset) /
         mega_texture_size;
}

vec3 fetch_color(uint texture_index, vec3 otherwise) {
  if (texture_index == invalid_texture) {
    return otherwise;
  }

  vec2 coords = get_texture_coords(texture_index);
  return texture(u_mega_texture, coords).rgb;
}

void main(void) {
  MeshInfo mesh_info = minfo[ID];
  vec3 n = normalize(N) * (gl_FrontFacing ? 1.0f : -1.0f);

  vec3 base_color =
      fetch_color(mesh_info.base_color_texture_index, vec3(0.0f, 1.0f, 0.0f));
  vec3 r = fetch_color(mesh_info.roughness_texture_index, vec3(0.5f));

  O_color = vec4(base_color, 1.0f);
  O_normal = vec4(n * 0.5f + 0.5f, 1.0f);
  O_roughness = vec4(r.b, r.g, 0.0f, 1.0f);
}
