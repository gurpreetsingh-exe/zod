const char* g_flat_color_frag = R"(
#version 450

out vec4 color;

uniform vec3 u_color;

void main() {
    color = vec4(u_color, 1.0f);
}
)";
