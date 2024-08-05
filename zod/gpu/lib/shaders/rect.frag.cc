const char* g_rect_frag = R"(
#version 450

in vec2 uv;
out vec4 color;

const vec3 base = vec3(30., 30., 46.) / 255.;
const vec3 mantle = vec3(24., 24., 37.) / 255.;
const vec3 surface0 = vec3(49., 50., 68.) / 255.;

uniform vec3 u_color;

void main() {
    color = vec4(u_color, 1.0f);
}
)";
