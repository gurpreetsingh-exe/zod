const char* g_round_panel = R"(
#version 450

in vec2 uv;
out vec4 color;

const vec3 base = vec3(30., 30., 46.) / 255.;
const vec3 mantle = vec3(24., 24., 37.) / 255.;
const vec3 surface0 = vec3(49., 50., 68.) / 255.;

uniform vec3 u_color;

void main() {
    float t = length(max(abs(uv * 2.0f - 1.0f) - 0.5f, 0.0f));
    t = smoothstep(0.38, 0.42, t);
    color = vec4(mix(u_color, mantle, t), t);
}
)";
