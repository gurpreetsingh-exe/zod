#version 460

in vec3 P;
in vec3 N;
in vec2 uv;
flat in uint ID;

layout(std430, binding = 6) buffer Camera {
  mat4 view;
  mat4 projection;
  mat4 inv_view;
  mat4 inv_projection;
  vec4 rd;
  vec4 ro;
};

struct MeshInfo {
  uint matrix_index;
  uint base_color_texture_index;
  uint normal_texture_index;
};

struct TextureInfo {
  vec2 offset;
  vec2 size;
};

layout(std430, binding = 3) buffer meshInfo { MeshInfo minfo[]; };
layout(std430, binding = 4) buffer textureInfo { TextureInfo tinfo[]; };

layout(location = 0) out vec4 O_color;
layout(location = 1) out vec4 O_normal;

uniform sampler2D u_mega_texture;

vec2 get_texture_coords(uint texture_index) {
  TextureInfo texture_info = tinfo[texture_index];
  vec2 mega_texture_size = vec2(textureSize(u_mega_texture, 0));
  return ((fract(uv) * texture_info.size) + texture_info.offset) /
         mega_texture_size;
}

void main(void) {
  MeshInfo mesh_info = minfo[ID];
  vec3 n = normalize(N);

  vec4 base_color = texture(
      u_mega_texture, get_texture_coords(mesh_info.base_color_texture_index));
  vec3 normal = texture(u_mega_texture,
                        get_texture_coords(mesh_info.normal_texture_index))
                    .rgb;

  O_color = vec4(base_color.rgb, 1.0f);
  O_normal = vec4(n, 1.0f);
}
