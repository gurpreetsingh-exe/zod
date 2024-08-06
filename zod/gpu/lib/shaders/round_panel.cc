const char* g_round_panel = R"(
#version 450

in vec2 uv;
out vec4 color;

const vec3 mantle = vec3(0.07f, 0.08f, 0.08f);

uniform vec3 u_color;

void main() {
    float t = length(max(abs(uv * 2.0f - 1.0f) - 0.5f, 0.0f));
    t = smoothstep(0.38, 0.42, t);
    color = vec4(mix(u_color, mantle, t), t);
}
)";
