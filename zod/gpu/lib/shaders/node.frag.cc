const char* g_node_frag = R"(
#version 450

in vec2 uv;
flat in uint instance;
layout (location = 0) out vec4 color;
layout (location = 1) out vec4 id;
uniform uint u_active;

struct Node {
    vec2 offset;
    uint id;
    uint extra;
};

layout (std430, binding = 1) buffer Nodes {
  Node nodes[];
};

void main() {
    float steprange = 0.005;
    vec2 coord = (uv - 0.5) * 2.0f;
    float border = 0.15f;
    float node = length(max(abs(coord) + border - vec2(0.9, 0.3), 0.0f)) - border;
    float node_mask = smoothstep(-steprange, steprange, node);
    vec3 c = mix(vec3(0.1), vec3(0.2), node_mask);
    float socket_radius = 0.08;
    float socket = length(abs(coord) - vec2(0.0f, 0.3f + socket_radius)) - socket_radius;
    float socket_mask = smoothstep(-steprange, steprange, socket);
    c = mix(vec3(0.8f), c, socket_mask);
    float mask = 1 - min(node_mask, socket_mask);
    color = vec4(c, mask);
    steprange *= 5.0f;
    float selection_mask = smoothstep(steprange, -steprange, node - 0.03);
    // c = mix(vec3(0.9294, 0.4431, 0.1411), c, mask);
    uint i = nodes[instance].id;
    if (u_active == i) {
        c = mix(vec3(0.0, 0.45, 0.82), c, mask);
        color = vec4(c, max(selection_mask, 1 - socket_mask));
    } else {
        color = vec4(c, mask);
    }

    float r = i & 0x000000ff;
    float g = (i & 0x0000ff00) >> 8;
    float b = (i & 0x00ff0000) >> 16;
    id = vec4(r, g, b, 255) / 255 * mask;
}
)";
