const char* g_rect = R"(
#version 450

layout (packed) uniform UIUbo {
    mat4 view_projection_mat;
    float width;
    float height;
};

in vec2 uv;
out vec4 color;

uniform int u_border;

void main() {
    float aspect = height / width;
    vec2 size = vec2(width, height);
    vec2 center = (uv - 0.5) * size;
    vec2 c = abs(center) - size * 0.5;
    c += 0.2 * u_border;
    float t = min(step(c.x, 0.0f), step(c.y, 0.0f));
    color = vec4(vec3(0.2f), 1 - t);
}
)";
