#include "tonemapping.glsl"

in vec2 uv;

uniform sampler2D u_indirect;

layout(location = 0) out vec4 O_color;

const int Lod = 1;

#define SIGMA 10.0
#define BSIGMA 0.1
#define MSIZE 8

float normpdf(in float x, in float sigma) {
  return 0.39894 * exp(-0.5 * x * x / (sigma * sigma)) / sigma;
}

float normpdf3(in vec3 v, in float sigma) {
  return 0.39894 * exp(-0.5 * dot(v, v) / (sigma * sigma)) / sigma;
}

void main() {
#if 0
    vec2 size = vec2(textureSize(u_indirect, Lod));
  vec2 texelUV = uv * size;

  vec3 c = textureLod(u_indirect, uv, Lod).rgb;
  // declare stuff
  const int kSize = (MSIZE - 1) / 2;
  float kernel[MSIZE];
  vec3 final_colour = vec3(0.0);

  // create the 1-D kernel
  float Z = 0.0;
  for (int j = 0; j <= kSize; ++j) {
    kernel[kSize + j] = kernel[kSize - j] = normpdf(float(j), SIGMA);
  }

  vec3 cc;
  float factor;
  float bZ = 1.0 / normpdf(0.0, BSIGMA);

  // read out the texels
  for (int i = -kSize; i <= kSize; ++i) {
    for (int j = -kSize; j <= kSize; ++j) {
      cc = textureLod(u_indirect,
                   (texelUV + vec2(float(i), float(j))) /
                                        size, Lod)
               .rgb;
      factor =
          normpdf3(cc - c, BSIGMA) * bZ * kernel[kSize + j] * kernel[kSize + i];
      Z += factor;
      final_colour += factor * cc;
    }
  }
  O_color = vec4(tonemap(final_colour / Z), 1.0f);
#endif

  O_color = vec4(texture(u_indirect, uv).rgb, 1.0f);
}
