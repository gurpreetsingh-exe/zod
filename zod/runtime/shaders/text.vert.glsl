layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;

out vec2 uv;

uniform mat4 u_projection;

void main(void) {
  uv = a_uv;
  gl_Position = u_projection * vec4(a_position, 0.0f, 1.f);
}
