#include "buffers.glsl"

const vec3 positions[4] = vec3[](
    vec3(-1, -1, 0),
    vec3(-1, 1, 0),
    vec3(1, -1, 0),
    vec3(1, 1, 0)
    );

out vec2 V_tex_coords;

void translate(inout mat4 m, vec3 v) {
  m[3] = m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3];
}

void main() {
  vec3 position = positions[gl_VertexID];
  LightInfo info = light_info[gl_InstanceID + 1];
  mat4 position_matrix = matrix[info.index].model;
  vec3 offset = vec3(position_matrix[3]);
  mat4 rotation = inverse(mat4(mat3(view)));
  mat4 model = mat4(1.0f);
  translate(model, offset);
  model = model * rotation;
  gl_Position = projection * view * model * vec4(position * 0.4f, 1.f);
  V_tex_coords = position.xy * 0.5f + 0.5f;
};
