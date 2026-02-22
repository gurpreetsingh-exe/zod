in vec2 V_tex_coords;
out vec4 O_color;

uniform sampler2D u_texture;

void main() {
    O_color = texture2D(u_texture, V_tex_coords);
}
