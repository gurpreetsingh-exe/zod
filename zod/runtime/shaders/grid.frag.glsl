#version 450

layout(std140) uniform Camera {
  mat4 view_projection;
  vec4 direction;
};

in vec3 near_point;
in vec3 far_point;

out vec4 color;

uniform vec3 u_color;
const float width = 1;

vec4 grid(vec3 position, float scale) {
  vec2 coord =
      position.xy *
      scale; // use the scale variable to set the distance between the lines
  vec2 derivative = fwidth(coord);
  vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
  float line = min(grid.x, grid.y);
  vec2 dmin = min(derivative, 1);
  vec4 color = vec4(u_color, 1.0 - min(line, 1.0));
  if (position.x > -width * dmin.x && position.x < width * dmin.x) {
    color.y = 1.0;
  }
  if (position.y > -width * dmin.y && position.y < width * dmin.y) {
    color.x = 1.0;
  }
  return color;
}

float compute_depth(vec3 pos) {
  vec4 clip_space_pos = view_projection * vec4(pos, 1.0);
  return clip_space_pos.z / clip_space_pos.w;
}

const float near = 0.01;
const float far = 100.0;

float depth_fix_range(float depth) {
  return ((gl_DepthRange.diff * depth) + gl_DepthRange.near +
          gl_DepthRange.far) /
         2.0;
}

float compute_linear_depth(vec3 pos) {
  vec4 clip_space_pos = view_projection * vec4(pos, 1.0);
  float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0;
  float linera_depth =
      (2.0 * near * far) /
      (far + near -
       clip_space_depth *
           (far - near)); // get linear value between 0.01 and 100
  return linera_depth / far;
}

void main() {
  float t = -near_point.z / (far_point.z - near_point.z);
  vec3 position = near_point + t * (far_point - near_point);
  float depth = compute_depth(position);
  gl_FragDepth = depth_fix_range(depth) - 2.4e-6;

  float ln_depth = compute_linear_depth(position);
  float fading = max(0, (0.6 - ln_depth));

  vec4 small = grid(position, 0.5);
  vec4 large = grid(position, 0.1);
  vec3 c = small.rgb + large.rgb;
  float mask = min(small.w + large.w, 1);
  mask *= step(0, t) * fading;
  color = vec4(c.xyz, mask);
}
