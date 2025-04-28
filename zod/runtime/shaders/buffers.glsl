layout(std430, binding = 6) buffer Camera {
  mat4 view;
  mat4 projection;
  mat4 inv_view;
  mat4 inv_projection;
  vec4 rd;
  vec4 ro;
};

struct Matrix {
  mat4 model;
  mat4 inv_model;
};

struct TextureInfo {
  vec2 offset;
  vec2 size;
};

struct LightInfo {
  uint index;
  float a;
  float b;
};

struct MeshInfo {
  uint matrix_index;
  uint base_color_texture_index;
  uint normal_texture_index;
  uint roughness_texture_index;
};

layout(std430, binding = 0) buffer vertexNormal { float buffer_normal[]; };
layout(std430, binding = 1) buffer modelMatrix { Matrix matrix[]; };
layout(std430, binding = 2) buffer vertexPosition { float buffer_position[]; };
layout(std430, binding = 3) buffer meshInfo { MeshInfo minfo[]; };
layout(std430, binding = 4) buffer textureInfo { TextureInfo tinfo[]; };
layout(std430, binding = 5) buffer vertexUV { vec2 buffer_uv[]; };
layout(std430, binding = 7) buffer lightInfo { LightInfo light_info[]; };
