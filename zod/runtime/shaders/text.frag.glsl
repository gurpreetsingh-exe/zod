in vec2 uv;
out vec4 color;

uniform sampler2D u_texture;

float median(float r, float g, float b) {
  return max(min(r, g), min(max(r, g), b));
}

float screen_px_range() {
  const float px_range = 4.0;
  vec2 unit_range = vec2(px_range) / vec2(textureSize(u_texture, 0));
  vec2 screen_tex_size = vec2(1.0) / fwidth(uv);
  return max(0.5 * dot(unit_range, screen_tex_size), 1.0);
}

void main() {
  vec3 c = texture(u_texture, uv).rgb;
  float distance = median(c.r, c.g, c.b);
  float screen_px_distance = screen_px_range() * (distance - 0.5);
  float alpha = clamp(screen_px_distance + 0.5, 0.0, 1.0);
  color = vec4(vec3(1), alpha);
}
