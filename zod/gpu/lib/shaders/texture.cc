const char* g_texture = R"(
#version 450

in vec2 uv;
out vec4 color;

uniform sampler2D u_texture;

void main() {
    color = vec4(texture2D(u_texture, uv).rgb, 1.0f);
}
)";
