layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;

layout(std140, binding = 1) uniform Camera {
  mat4 view_projection;
  vec4 direction;
};

out vec4 v_color;

void main() {
  gl_Position = view_projection * vec4(position, 0.0f, 1.0f);
  v_color = color;
};
