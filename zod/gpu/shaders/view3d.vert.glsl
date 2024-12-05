#version 450

layout(location = 0) in vec3 position;
out vec3 P;

layout(std140) uniform Camera {
  mat4 view_projection;
  vec4 direction;
};

layout(std430, binding = 2) buffer vertexPosition { float buffer_position[]; };

void main(void) {
  // P = position;
  // gl_Position = view_projection * vec4(position, 1.f);
  int id = gl_VertexID * 3;
  vec3 p = vec3(buffer_position[id], buffer_position[id + 1],
                buffer_position[id + 2]);
  P = p;
  gl_Position = view_projection * vec4(p, 1.f);
}
