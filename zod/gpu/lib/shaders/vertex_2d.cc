const char* g_vertex_2d = R"(
#version 450

layout (packed) uniform UIUbo {
    mat4 view_projection_mat;
};

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec2 offset;
out vec2 uv;

uniform int u_border;

void main() {
  uv = a_uv;
  float size = float(u_border);
  if (gl_InstanceID == 0) {
      gl_Position = view_projection_mat * vec4(position * size + offset, 0.f, 1.f);
      uv *= 0.25f;
  } else if (gl_InstanceID == 1) {
      gl_Position = view_projection_mat * vec4(position * size + offset - vec2(size, 0.0f), 0.f, 1.f);
      uv *= 0.25f;
      uv.x += 0.75f;
  } else if (gl_InstanceID == 2) {
      gl_Position = view_projection_mat * vec4(position * size + offset - vec2(0.0f, size), 0.f, 1.f);
      uv *= 0.25f;
      uv.y += 0.75f;
  } else {
      gl_Position = view_projection_mat * vec4(position * size + offset - vec2(size, size), 0.f, 1.f);
      uv *= 0.25f;
      uv += 0.75f;
  }
}
)";
