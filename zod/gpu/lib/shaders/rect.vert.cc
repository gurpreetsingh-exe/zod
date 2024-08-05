const char* g_rect_vert = R"(
#version 450

layout (packed) uniform UIUbo {
    mat4 view_projection_mat;
};

layout (location = 0) in vec2 position;
out vec2 uv;

uniform int u_border;

void main() {
  vec2 p = position;
  float padding = 0.2 * u_border;
  int id = gl_VertexID % 4;
  if (id == 0) {
    uv = vec2(0);
    p += vec2(padding);
  } else if (id == 1) {
    uv = vec2(1, 0);
    p += vec2(-padding, padding);
  } else if (id == 2) {
    uv = vec2(0, 1);
    p += vec2(padding, -padding);
  } else {
    uv = vec2(1, 1);
    p += vec2(-padding);
  }
  gl_Position = view_projection_mat * vec4(p, 0.f, 1.f);
}
)";
