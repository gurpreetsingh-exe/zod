const char* g_uv = R"(
#version 450

in vec2 uv;
out vec4 color;

void main() {
    color = vec4(uv, 0.0f, 1.0f);
}
)";
