const char* g_rect_frag = R"(
#version 450

in vec2 uv;
out vec4 color;

uniform vec3 u_color;
uniform int u_width;
uniform int u_height;
uniform int u_border;
uniform int u_padding;

void main() {
    if (u_border == 0) {
        vec2 size = vec2(u_width, u_height);
        vec2 c = abs(uv - size * 0.5) - size * 0.5 + u_padding;
        vec2 c0 = max(c, 0.0f);
        float c1 = step(length(c0), 0.01);
        color = vec4(u_color, c1);
        return;
    }
    vec2 size = vec2(u_width, u_height);
    vec2 c = abs(uv - size * 0.5) - size * 0.5 + u_border + u_padding;
    vec2 c0 = max(c, 0.0f);
    float c1 = u_border - length(c0);
    float c2 = clamp(c1, 0, 1);
    float t = -min(c.x, c.y);
    t = step(0, t);
    t = max(step(0.01, c2) * t, c2);
    color = vec4(u_color, t);
}
)";
