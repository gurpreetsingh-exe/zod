#version 450

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;

out vec2 uv;

layout(std140, binding = 1) uniform Camera {
  mat4 view_projection;
  vec4 direction;
};

void main(void) {
  uv = a_uv;
  gl_Position = view_projection * vec4(a_position, 0.0f, 1.f);
}
