#include "buffers.glsl"

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

vec3 F_Schlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 get_color(vec3 P, vec3 v, vec3 base_color, vec3 n, float metallic, float roughness) {
  vec3 base_reflectivity = mix(vec3(0.04f), base_color, metallic);

  vec3 Lo = vec3(0.0f);
  for (uint i = 1; i < light_info[0].index + 1; ++i) {
    LightInfo linfo = light_info[i];
    uint light_index = linfo.index;
    Matrix light_matrix = matrix[light_index];
    vec3 light_position = vec3(light_matrix.model[3]);
    vec3 L = normalize(light_position - P);
    vec3 H = normalize(v + L);
    float distance = length(light_position - P);
    float linear = linfo.a;
    float quadratic = linfo.b;
    float attenuation = 1.0f / (1.0f + linear * distance + quadratic * distance * distance);
    vec3 radiance = vec3(10.0f) * attenuation;

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

    Lo += (kD * base_color / PI + specular) * radiance * NoL;
  }

  vec3 ambient = base_color * 0.03f;
  return ambient + Lo;
}
