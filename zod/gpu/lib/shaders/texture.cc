const char* g_texture = R"(
#version 450

in vec2 uv;
out vec4 color;

uniform sampler2D u_texture;

void main() {
    vec2 flipped = uv;
    flipped.y = 1 - flipped.y;
    color = vec4(texture2D(u_texture, flipped).rgb, 1.0f);
}
)";
