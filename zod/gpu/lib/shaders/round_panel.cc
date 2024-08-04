const char* g_round_panel = R"(
#version 450

in vec2 uv;
out vec4 color;

void main() {
    float t = step(length(max(abs(uv * 2.0f - 1.0f) - 0.5f, 0.0f)), 0.4);
    color = vec4(vec3(0.2f), 1 - t);
}
)";
