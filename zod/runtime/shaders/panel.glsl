layout(packed) uniform UIUbo {
  int width;
  int height;
};

in vec2 uv;
in vec2 size;
out vec4 color;

const vec3 base = vec3(30., 30., 46.) / 255.;
const vec3 mantle = vec3(24., 24., 37.) / 255.;
const vec3 surface0 = vec3(49., 50., 68.) / 255.;

void main() {
  float border = 0.008f;
  vec2 aspect = vec2(1.0f, float(height) / float(width));
  vec2 hsize = size * 0.5;
  vec2 center = (uv - 0.5) * size * aspect;
  float c =
      length(max((abs(center) - hsize * 0.99 * aspect) + border * aspect, 0));
  color = vec4(vec3(step(c, border)), 1.0f);
  // color = vec4(center, 0.0f, 1.0f);
}
