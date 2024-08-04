const char* g_round_cube = R"(
#version 450

in vec2 uv;
out vec4 color;

void main() {
  float c = length(max(abs(uv * 2.0f - 1.0f) - 0.5f, 0.0f));
  // 0.4 + 0.4 = 0.8
  // 1 - 0.8 =
  // 0.2 * 10 (from vertex_2d) = 2 is the padding
  c = step(c, 0.4f);
  color = vec4(c, c, c, 1.0f);
}
)";
