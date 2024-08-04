const char* g_round_panel = R"(
#version 450

in vec2 uv;
out vec4 color;

uniform sampler2D u_round_cube;

void main() {
    float t = texture2D(u_round_cube, uv).r;
    color = vec4(vec3(1.0f) * t, 1.f);
}
)";
