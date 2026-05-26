layout(std430, binding = 2) readonly buffer Color { vec4 g_color[]; };

in vec2 v_texcoords;

out vec4 color;

uniform sampler2D u_texture;

void main(void) {
  vec4 c = texture(u_texture, v_texcoords);
  if (c.a < 0.0001f) {
    discard;
  }

  color = c * g_color[gl_PrimitiveID];
}
