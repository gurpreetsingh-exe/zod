#include "buffers.glsl"

in vec3 P;
in vec3 N;
in vec2 uv;
flat in uint ID;

layout(location = 0) out vec4 O_color;
layout(location = 1) out vec4 O_normal;
layout(location = 2) out vec4 O_roughness;

uniform sampler2D u_mega_texture;

vec2 get_texture_coords(uint texture_index) {
  TextureInfo texture_info = tinfo[texture_index];
  vec2 mega_texture_size = vec2(textureSize(u_mega_texture, 0));
  return ((fract(uv) * texture_info.size) + texture_info.offset) /
         mega_texture_size;
}

void main(void) {
  MeshInfo mesh_info = minfo[ID];
  vec3 n = N;

  vec4 base_color = texture(
      u_mega_texture, get_texture_coords(mesh_info.base_color_texture_index));
  vec3 normal = texture(u_mega_texture,
                        get_texture_coords(mesh_info.normal_texture_index))
                    .rgb;
  vec3 r = texture(u_mega_texture,
                        get_texture_coords(mesh_info.roughness_texture_index))
                    .rgb;

  O_color = vec4(base_color.rgb, 1.0f);
  O_normal = vec4(n, 1.0f);
  O_roughness = vec4(r.b, r.g, 0.0f, 1.0f);
}
