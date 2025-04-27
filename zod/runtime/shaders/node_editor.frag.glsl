in vec2 uv;
layout(location = 0) out vec4 color;
layout(location = 1) out uint pixel;

void main() {
  vec3 surface0 = vec3(0.07f, 0.08f, 0.08f);
  color = vec4(surface0, 1.0f);
  pixel = 0;
}
