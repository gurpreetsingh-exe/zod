#version 450

#define MATH_PI 3.1415926535897932384626433832795
#define MATH_INV_PI (1.0 / MATH_PI)

in vec2 uv;
out vec4 color;

uniform int u_current_face;
uniform sampler2D u_panorama;

vec3 uv_to_xyz(int face, vec2 uv) {
  if (face == 0) {
    return vec3(1.f, uv.y, -uv.x);
  } else if (face == 1) {
    return vec3(-1.f, uv.y, uv.x);
  } else if (face == 2) {
    return vec3(uv.x, -1.f, uv.y);
  } else if (face == 3) {
    return vec3(uv.x, 1.f, -uv.y);
  } else if (face == 4) {
    return vec3(uv.x, uv.y, 1.f);
  } else {
    return vec3(-uv.x, uv.y, -1.f);
  }
}

vec2 dir_to_uv(vec3 dir) {
  return vec2(0.5f + 0.5f * atan(dir.y, dir.x) / MATH_PI,
              acos(dir.z) / MATH_PI);
}

vec3 panorama_to_cubemap(int face, vec2 coords) {
  vec2 coords_new = coords * 2.0 - 1.0;
  vec3 scan = uv_to_xyz(face, coords_new);
  vec3 direction = normalize(scan);
  vec2 src = dir_to_uv(direction);
  return texture(u_panorama, src).rgb;
}

void main() {
  color = vec4(0.0, 0.0, 0.0, 1.0);
  color.rgb = panorama_to_cubemap(u_current_face, uv);
}
