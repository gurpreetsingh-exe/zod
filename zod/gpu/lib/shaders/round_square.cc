const char* g_round_cube = R"(
#version 450

in vec2 uv;
out vec4 color;

void main() {
  float c = length(max(abs(uv * 2.0f - 1.0f) - 0.5f, 0.0f));
  c = step(c, 0.4f);
  color = vec4(c, c, c, 1.0f);
}
)";
