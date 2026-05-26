#include "brdf.glsl"
#include "tonemapping.glsl"

in vec2 uv;

uniform sampler2D u_albedo;
uniform sampler2D u_position;
uniform sampler2D u_normal;
uniform sampler2D u_roughness;
uniform sampler2D u_depth;

layout(location = 0) out vec4 O_color;

const float hitThickness = 1.0f;
const float pi = 3.14159f;
const float twoPi = pi * 2.0f;
const float sliceCount = 4;
const float sampleCount = 4;
const float sampleRadius = 4.0f;
const float halfPi = 0.5 * pi;
const int Lod = 2;

float randf(int x, int y) {
  return mod(52.9829189 *
                 mod(0.06711056 * float(x) + 0.00583715 * float(y), 1.0),
             1.0);
}

// https://graphics.stanford.edu/%7Eseander/bithacks.html
uint bitCount(uint value) {
  value = value - ((value >> 1u) & 0x55555555u);
  value = (value & 0x33333333u) + ((value >> 2u) & 0x33333333u);
  return ((value + (value >> 4u) & 0xF0F0F0Fu) * 0x1010101u) >> 24u;
}

const uint sectorCount = 32u;
uint updateSectors(float minHorizon, float maxHorizon, uint outBitfield) {
  uint startBit = uint(minHorizon * float(sectorCount));
  uint horizonAngle =
      uint(ceil((maxHorizon - minHorizon) * float(sectorCount)));
  uint angleBit = horizonAngle > 0u
                      ? uint(0xFFFFFFFFu >> (sectorCount - horizonAngle))
                      : 0u;
  uint currentBitfield = angleBit << startBit;
  return outBitfield | currentBitfield;
}

// https://stackoverflow.com/a/32246825
// this is supposed to get the world position from the depth buffer
vec3 world_pos_from_depth(float depth) {
  float z = depth * 2.0 - 1.0;

  vec4 clip_space_pos = vec4(uv * 2.0 - 1.0, z, 1.0);
  vec4 view_space_pos = inv_projection * clip_space_pos;

  // Perspective division
  view_space_pos /= view_space_pos.w;

  vec4 world_pos = inv_view * view_space_pos;

  return world_pos.xyz;
}

float noise(vec2 c) {
  return fract(sin(dot(c.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec4 get_visibility(vec2 fragUV) {
  uint indirect = 0u;
  uint occlusion = 0u;

  float visibility = 0.0;
  vec3 lighting = vec3(0.0);
  vec2 frontBackHorizon = vec2(0.0);
  vec2 screenSize = vec2(textureSize(u_albedo, Lod));
  vec2 aspect = screenSize.yx / screenSize.x;
  vec3 position = textureLod(u_position, fragUV, Lod).rgb;
  // vec3 position = world_pos_from_depth(texture(u_depth, fragUV).x);
  vec3 camera = normalize(-position);
  vec3 normal = normalize(textureLod(u_normal, fragUV, Lod + 3).rgb);
  // position *= 0.9999;

  float sliceRotation = twoPi / (sliceCount - 1.0);
  float sampleScale = (-sampleRadius * projection[0][0]) / position.z;
  float sampleOffset = 0.01;
  float jitter = randf(int(gl_FragCoord.x), int(gl_FragCoord.y)) - 0.5;

  for (float slice = 0.0; slice < sliceCount + 0.5; slice += 1.0) {
    float phi = sliceRotation * (slice + jitter) + pi;
    vec2 omega = vec2(cos(phi), sin(phi));
    vec3 direction = vec3(omega.x, omega.y, 0.0);
    vec3 orthoDirection = direction - dot(direction, camera) * camera;
    vec3 axis = cross(direction, camera);
    vec3 projNormal = normal - axis * dot(normal, axis);
    float projLength = length(projNormal);

    float signN = sign(dot(orthoDirection, projNormal));
    float cosN = clamp(dot(projNormal, camera) / projLength, 0.0, 1.0);
    float n = signN * acos(cosN);

    for (float currentSample = 0.0; currentSample < sampleCount + 0.5;
         currentSample += 1.0) {
      float sampleStep = (currentSample + jitter) / sampleCount + sampleOffset;
      vec2 sampleUV = fragUV - sampleStep * sampleScale * omega * aspect;
      vec3 samplePosition = textureLod(u_position, sampleUV, Lod).rgb;
      vec3 sampleNormal =
          normalize(textureLod(u_normal, sampleUV, Lod + 3).rgb);
      // vec3 sampleLight = vec3(1.0f);
      vec3 sampleLight = textureLod(u_albedo, sampleUV, Lod + 1).rgb;
      vec3 sampleDistance = samplePosition - position;
      float sampleLength = length(sampleDistance);
      vec3 sampleHorizon = sampleDistance / sampleLength;

      frontBackHorizon.x = dot(sampleHorizon, camera);
      frontBackHorizon.y =
          dot(normalize(sampleDistance - camera * hitThickness), camera);

      frontBackHorizon = acos(frontBackHorizon);
      frontBackHorizon = clamp((frontBackHorizon + n + halfPi) / pi, 0.0, 1.0);

      indirect = updateSectors(frontBackHorizon.x, frontBackHorizon.y, 0u);
      lighting +=
          (1.0 - float(bitCount(indirect & ~occlusion)) / float(sectorCount)) *
          sampleLight * clamp(dot(normal, sampleHorizon), 0.0, 1.0) *
          clamp(dot(sampleNormal, -sampleHorizon), 0.0, 1.0);
      occlusion |= indirect;
    }
    visibility += 1.0 - float(bitCount(occlusion)) / float(sectorCount);
  }

  visibility /= sliceCount;
  lighting /= sliceCount;

  return vec4(lighting, visibility);
}

void main() {
  // {
  float depth = texture(u_depth, uv).x;
  vec3 P = world_pos_from_depth(depth);
  vec3 v = normalize(ro.xyz - P);

  vec3 base_color = texture(u_albedo, uv).rgb;
  vec3 normal = texture(u_normal, uv).rgb;
  vec3 r = texture(u_roughness, uv).rgb;
  vec3 n = normal;

  float metallic = r.x;
  float roughness = r.y;
  vec3 color;

  // vec3 random = vec3(noise(uv), noise(uv + 1.0f), 0.0f);
  // vec3 tangent = normalize(random - normal * dot(random, normal));
  // vec3 binormal = cross(normal, tangent);
  // mat3 tbn = mat3(tangent, binormal, normal);
  // }

  color = get_color(P, v, base_color, n, metallic, roughness);
  // vec3 color = vec3(base_color);

  // vec4 gi = get_visibility(uv);
  // color = gi.rgb * base_color + vec3(0.04f) * gi.a;

  vec3 origin = P;
  vec3 random = vec3(noise(uv), noise(uv + 1.0f), noise(uv + 2.0f));
  vec3 direction = normalize(n);

  origin = origin + direction * depth;
  vec4 H0 = projection * vec4(origin, 1.0f);

  // vec2 sampleUV = uv - sampleStep * sampleScale * omega * aspect;
  // vec3 samplePosition = textureLod(u_position, sampleUV, Lod).rgb;
  // color = world_pos_from_depth(texture(u_depth, H0.xy / H0.w).x);
  // color = texture(u_albedo, H0.xy / H0.w).rgb;

  O_color = vec4(tonemap(color), 1.0f);
  // O_color = vec4(color, 1.0f);
}
