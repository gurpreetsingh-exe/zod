const char* g_view3d_frag = R"(
#version 450

in vec3 pos;
out vec4 color;

void main(void) {
  color = vec4(pos, 1.0f);
}
)";
