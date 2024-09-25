const char* g_fragment2d = R"(
#version 450

in vec4 v_color;
out vec4 color;

void main() {
    color = v_color;
};
)";
