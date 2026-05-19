in vec2 uv;
out vec4 O_color;

uniform sampler2D u_texture;

void main() { O_color = texture2D(u_texture, uv); }
