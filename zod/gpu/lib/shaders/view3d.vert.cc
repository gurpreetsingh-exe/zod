const char* g_view3d_vert = R"(
#version 450

layout (location = 0) in vec3 position;
out vec3 P;

layout(std140) uniform Camera {
    mat4 view_projection;
    vec4 direction;
};

void main(void) {
  P = position;
  gl_Position = view_projection * vec4(position, 1.f);
}
)";
