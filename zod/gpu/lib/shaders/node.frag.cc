const char* g_node_frag = R"(
#version 450

in vec2 uv;
out vec4 color;

void main() {
    vec2 size = vec2(84, 100);
    vec2 coord = (uv - 0.5) * size;
    float border = 2.0f;
    float header = coord.y - 33;
    float sep = (border * 0.5) - abs(header);
    float t = max(length(max(abs(coord) + border - size * 0.5, 0.0f)), sep);
    vec3 background = mix(vec3(0.1), vec3(0.25), step(0.001, header));
    vec3 c = mix(background, vec3(0.2), step(0.001, t));
    color = vec4(c, 1.0f);
    // color = vec4(t * vec3(1), 1.0f);
}
)";
