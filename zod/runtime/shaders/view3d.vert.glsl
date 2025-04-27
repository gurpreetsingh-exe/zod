out vec3 P;
out vec3 N;
out uint ID;

layout(std140) uniform Camera {
  mat4 view_projection;
  vec4 direction;
};

struct MeshInfo {
  uint matrix_index;
  uint _unused_base_color_texture_index;
};

layout(std430, binding = 0) buffer vertexNormal { float buffer_normal[]; };
layout(std430, binding = 1) buffer modelMatrix { mat4 model[]; };
layout(std430, binding = 2) buffer vertexPosition { float buffer_position[]; };
layout(std430, binding = 3) buffer meshInfo { MeshInfo info[]; };

void main(void) {
  int id = gl_VertexID * 3;
  vec3 p = vec3(buffer_position[id], buffer_position[id + 1],
                buffer_position[id + 2]);
  vec3 n =
      vec3(buffer_normal[id], buffer_normal[id + 1], buffer_normal[id + 2]);
  P = p;
  N = n;
  ID = gl_DrawID;
  MeshInfo inf = info[gl_DrawID];
  gl_Position = view_projection * model[inf.matrix_index] * vec4(p, 1.f);
}
