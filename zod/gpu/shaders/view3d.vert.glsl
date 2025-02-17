#version 460

out vec3 P;

layout(std140) uniform Camera {
  mat4 view_projection;
  vec4 direction;
};

layout(std430, binding = 1) buffer modelMatrix { mat4 model[]; };
layout(std430, binding = 2) buffer vertexPosition { float buffer_position[]; };

void main(void) {
  int id = gl_VertexID * 3;
  vec3 p = vec3(buffer_position[id], buffer_position[id + 1],
                buffer_position[id + 2]);
  P = p;
  gl_Position = view_projection * model[gl_DrawID] * vec4(p, 1.f);
}
