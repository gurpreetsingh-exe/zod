#include "buffers.glsl"

out vec3 P;
out vec3 N;
out vec2 uv;
out uint ID;

void main(void) {
  int id = gl_VertexID * 3;
  vec3 p = vec3(buffer_position[id], buffer_position[id + 1],
                buffer_position[id + 2]);
  vec3 n =
      vec3(buffer_normal[id], buffer_normal[id + 1], buffer_normal[id + 2]);
  MeshInfo inf = minfo[gl_DrawID];
  Matrix mtx = matrix[inf.matrix_index];
  P = vec3(mtx.model * vec4(p, 1.0f));
  N = normalize(mat3(mtx.inv_model) * n);
  uv = buffer_uv[gl_VertexID];
  ID = gl_DrawID;
  gl_Position = projection * view * mtx.model * vec4(p, 1.f);
}
