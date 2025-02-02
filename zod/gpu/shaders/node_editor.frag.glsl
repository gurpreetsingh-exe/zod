#version 450

in vec2 uv;
out vec4 color;

uniform int u_width;
uniform int u_height;

void main() {
  vec3 surface0 = vec3(0.07f, 0.08f, 0.08f);
  color = vec4(surface0, 1.0f);
}
