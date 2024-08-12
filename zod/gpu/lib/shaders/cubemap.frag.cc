const char* g_cubemap_frag = R"(
#version 450

in vec3 P;
out vec4 color;

void main() {
    color = vec4(0.05 * vec3(1), 1.0f);
}
)";
