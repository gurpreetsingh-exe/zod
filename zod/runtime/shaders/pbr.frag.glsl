#version 460

in vec2 uv;

layout(std430, binding = 6) buffer Camera {
  mat4 view;
  mat4 projection;
  mat4 inv_view;
  mat4 inv_projection;
  vec4 rd;
  vec4 ro;
};

uniform sampler2D u_albedo;
uniform sampler2D u_normal;
uniform sampler2D u_depth;

layout(location = 0) out vec4 O_color;

////////////////////////////////////////////////////
/// PBR ////////////////////////////////////////////
////////////////////////////////////////////////////

#define PI 3.14159f

float D_GGX(float NoH, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float d = NoH * NoH * (a2 - 1.0f) + 1.0f;
  d = PI * d * d;
  return a2 / max(d, 0.000001f);
}

float GeometrySchlickGGX(float NoV, float roughness) {
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;

  float nom = NoV;
  float denom = NoV * (1.0 - k) + k;

  return nom / denom;
}

float GeometrySmith(float NoV, float NoL, float roughness) {
  float ggx2 = GeometrySchlickGGX(NoV, roughness);
  float ggx1 = GeometrySchlickGGX(NoL, roughness);
  return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 F_Schlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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

////////////////////////////////////////////////////
/// PBR END ////////////////////////////////////////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
/// TONEMAPPING ////////////////////////////////////
////////////////////////////////////////////////////

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

////////////////////////////////////////////////////
/// TONEMAPPING END ////////////////////////////////
////////////////////////////////////////////////////

void main() {
  vec3 P = world_pos_from_depth(texture(u_depth, uv).x);
  vec3 v = normalize(ro.xyz - P);
  // vec3 n = normalize(N);

  vec3 base_color = texture(u_albedo, uv).rgb;
  vec3 normal = texture(u_normal, uv).rgb;
  vec3 n = normal;

  float metallic = 0.0f;
  float roughness = 0.5f;

  vec3 base_reflectivity = mix(vec3(0.04f), base_color, metallic);

  vec3 light_position = vec3(0.0f, 0.0f, 5.0f);
  vec3 L = normalize(light_position - P);
  vec3 H = normalize(v + L);
  float distance = length(light_position - P);
  float attenuation = 1.0f / (distance * distance);
  vec3 radiance = vec3(20.0f) * attenuation;

  float NoV = max(dot(n, v), 0.000001f);
  float NoL = max(dot(n, L), 0.000001f);
  float HoV = max(dot(H, v), 0.000001f);
  float NoH = max(dot(n, H), 0.000001f);

  float D = D_GGX(NoH, roughness);
  float G = GeometrySmith(NoV, NoL, roughness);
  vec3 F = F_Schlick(HoV, base_reflectivity);
  vec3 specular = D * G * F;
  specular /= 4.0f * NoV * NoL;

  vec3 kD = vec3(1.0f) - F;
  kD *= 1.0f - metallic;

  vec3 Lo = vec3(0.0f);
  Lo += (kD * base_color / PI + specular) * radiance * NoL;

  vec3 ambient = base_color * 0.03f;
  O_color = vec4(tonemap(ambient + Lo), 1.0f);
}
