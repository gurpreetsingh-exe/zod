in vec2 uv;
out vec4 color;

uniform sampler2D u_texture;

void main() {
  float d = texture(u_texture, uv).r;
  color = vec4(vec3(1), d * 1.25f);
}
