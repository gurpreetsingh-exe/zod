#version 460

out vec3 P;
out vec3 N;
out vec2 uv;
out uint ID;

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

layout(std430, binding = 0) buffer vertexNormal { float buffer_normal[]; };
layout(std430, binding = 1) buffer modelMatrix { mat4 model[]; };
layout(std430, binding = 2) buffer vertexPosition { float buffer_position[]; };
layout(std430, binding = 3) buffer meshInfo { MeshInfo minfo[]; };
layout(std430, binding = 5) buffer vertexUV { vec2 buffer_uv[]; };

void main(void) {
  int id = gl_VertexID * 3;
  vec3 p = vec3(buffer_position[id], buffer_position[id + 1],
                buffer_position[id + 2]);
  vec3 n =
      vec3(buffer_normal[id], buffer_normal[id + 1], buffer_normal[id + 2]);
  MeshInfo inf = minfo[gl_DrawID];
  mat4 m = model[inf.matrix_index];
  P = vec3(m * vec4(p, 1.0f));
  N = vec3(m * vec4(n, 1.0f));
  uv = buffer_uv[gl_VertexID];
  ID = gl_DrawID;
  gl_Position = projection * view * m * vec4(p, 1.f);
}
