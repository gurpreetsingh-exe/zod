const char* g_view3d_frag = R"(
#version 450

// adapted from Blender's workbench engine
// https://projects.blender.org/blender/blender/src/commit/30038f17105a4c3d2839b9f166995ae91ec12d35/source/blender/draw/engines/workbench/shaders/workbench_world_light_lib.glsl

in vec3 P;
out vec4 color;

layout(std140) uniform Camera {
    mat4 view_projection;
    vec4 direction;
};

layout(std430, binding = 0) buffer vertexNormals {
  float N[];
};

struct Light {
    vec3 direction;
    vec3 color;
    float w;
};

Light lights[4] = {
    Light(vec3(-0.854701, 0.111111, 0.507091), vec3(0.723042), 0.2f),
    Light(vec3(0.058607, -0.987943, -0.143295), vec3(0.063100, 0.069978, 0.067951), 0.719626f),
    Light(vec3(0.972202, 0.075846, -0.221518), vec3(0.157432, 0.163405, 0.214035), 0.281250f),
    Light(vec3(0), vec3(0), 0),
};

vec4 fast_rcp(vec4 v) {
  return intBitsToFloat(0x7eef370b - floatBitsToInt(v));
}

vec4 wrapped_lighting(vec4 NL, vec4 w) {
  vec4 w_1 = w + 1.0;
  vec4 denom = fast_rcp(w_1 * w_1);
  return clamp((NL + w) * denom, 0.0, 1.0);
}

void main(void) {
  int id = gl_PrimitiveID * 3;
  vec3 n = vec3(N[id], N[id + 1], N[id + 2]);
  float fresnel = dot(direction.xyz, n) * 0.5 + 0.5;

  vec3 ambient = vec3(0);
  vec3 diffuse_light = ambient;

  vec4 wrap = vec4(lights[0].w, lights[1].w, lights[2].w, lights[3].w);
  vec4 diff_NL = vec4(
    dot(lights[0].direction, n),
    dot(lights[1].direction, n),
    dot(lights[2].direction, n),
    dot(lights[3].direction, n)
  );

  vec4 diff_light = wrapped_lighting(diff_NL, wrap);

  diffuse_light += diff_light.x * lights[0].color.rgb;
  diffuse_light += diff_light.y * lights[1].color.rgb;
  diffuse_light += diff_light.z * lights[2].color.rgb;
  diffuse_light += diff_light.w * lights[3].color.rgb;
  diffuse_light *= (1 - fresnel);
  color = vec4(pow(diffuse_light, vec3(1.0f / 2.2f)), 1.0f);
}
)";
