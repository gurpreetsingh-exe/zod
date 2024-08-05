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
  uv = a_uv * 0.25f;
  float size = float(u_border);
  switch (gl_InstanceID % 4) {
    case 0: {
      gl_Position = view_projection_mat * vec4(position * size + offset, 0.f, 1.f);
      return;
    }
    case 1: {
      gl_Position = view_projection_mat * vec4(position * size + offset - vec2(size, 0.0f), 0.f, 1.f);
      uv.x += 0.75f;
      return;
    }
    case 2: {
      gl_Position = view_projection_mat * vec4(position * size + offset - vec2(0.0f, size), 0.f, 1.f);
      uv.y += 0.75f;
      return;
    }
    case 3: {
      gl_Position = view_projection_mat * vec4(position * size + offset - vec2(size, size), 0.f, 1.f);
      uv += 0.75f;
      return;
    }
  }
}
)";
