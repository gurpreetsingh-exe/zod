const float GAMMA = 2.2;
const float INV_GAMMA = 1.0 / GAMMA;

const mat3 ACES_input_mat = mat3(0.59719, 0.07600, 0.02840, 0.35458, 0.90834,
                                 0.13383, 0.04823, 0.01566, 0.83777);

const mat3 ACES_output_mat =
    mat3(1.60475, -0.10208, -0.00327, -0.53108, 1.10813, -0.07276, -0.07367,
         -0.00605, 1.07602);

// linear to sRGB approximation
// see http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
vec3 linear_to_sRGB(vec3 color) { return pow(color, vec3(INV_GAMMA)); }

// ACES filmic tone map approximation
// see https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
vec3 RRT_and_ODT_fit(vec3 color) {
  vec3 a = color * (color + 0.0245786) - 0.000090537;
  vec3 b = color * (0.983729 * color + 0.4329510) + 0.238081;
  return a / b;
}

vec3 tonemap_ACES_hill(vec3 color) {
  color = ACES_input_mat * color;
  color = RRT_and_ODT_fit(color);
  color = ACES_output_mat * color;
  color = clamp(color, 0.0, 1.0);
  return color;
}

vec3 tonemap(vec3 color) {
  color = tonemap_ACES_hill(color);
  return linear_to_sRGB(color);
}
