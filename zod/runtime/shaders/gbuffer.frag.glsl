#version 460

in vec3 P;
in vec3 N;
in vec2 uv;
flat in uint ID;
out vec4 color;

layout(std140) uniform Camera {
  mat4 view_projection;
  vec4 direction;
  vec4 camera_position;
};

struct MeshInfo {
  uint matrix_index;
  uint base_color_texture_index;
  uint normal_texture_index;
};

struct TextureInfo {
  vec2 offset;
  vec2 size;
};

layout(std430, binding = 3) buffer meshInfo { MeshInfo minfo[]; };
layout(std430, binding = 4) buffer textureInfo { TextureInfo tinfo[]; };

uniform sampler2D u_mega_texture;

vec2 get_texture_coords(uint texture_index) {
  TextureInfo texture_info = tinfo[texture_index];
  vec2 mega_texture_size = vec2(textureSize(u_mega_texture, 0));
  return ((fract(uv) * texture_info.size) + texture_info.offset) /
                mega_texture_size;
}

#define PI 3.14159f

float D_GGX(float NoH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float d = NoH * NoH * (a2 - 1.0f) + 1.0f;
    d = PI * d * d;
    return a2 / max(d, 0.000001f);
}

float GeometrySchlickGGX(float NoV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NoV;
    float denom = NoV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(float NoV, float NoL, float roughness)
{
    float ggx2 = GeometrySchlickGGX(NoV, roughness);
    float ggx1 = GeometrySchlickGGX(NoL, roughness);
    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 F_Schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main(void) {
  MeshInfo mesh_info = minfo[ID];
  vec3 v = normalize(camera_position.xyz - P);
  vec3 n = normalize(N);

  vec3 base_color = texture(u_mega_texture, get_texture_coords(mesh_info.base_color_texture_index)).rgb;
  vec3 normal = texture(u_mega_texture, get_texture_coords(mesh_info.normal_texture_index)).rgb;

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
  color = vec4(ambient + Lo, 1.0f);
}
