const char* g_view3d_vert = R"(
#version 450

layout (location = 0) in vec3 position;
out vec3 P;

uniform mat4 u_view_projection;

void main(void) {
  P = position;
  gl_Position = u_view_projection * vec4(position, 1.f);
}
)";
