const char* g_node_editor_frag = R"(
#version 450

in vec2 uv;
out vec4 color;

uniform int u_width;
uniform int u_height;

void main() {
    vec3 surface0 = vec3(0.15f, 0.16f, 0.17f);
    color = vec4(surface0, 1.0f);
}
)";
