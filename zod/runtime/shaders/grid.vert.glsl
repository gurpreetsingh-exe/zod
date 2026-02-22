#include "buffers.glsl"

out vec3 near_point;
out vec3 far_point;

vec3 grid_plane[6] = vec3[](vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
                            vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0));

vec3 unproject_point(float x, float y, float z) {
  mat4 inv = inv_view * inv_projection;
  vec4 point = inv * vec4(x, y, z, 1.0);
  return point.xyz / point.w;
}

void main(void) {
  vec3 P = grid_plane[gl_VertexID];
  near_point = unproject_point(P.x, P.y, 0.0f);
  far_point = unproject_point(P.x, P.y, 1.0f);
  gl_Position = vec4(P, 1.0f);
}
