#include "tonemapping.glsl"

in vec3 P;
out vec4 O_color;

uniform samplerCube u_cubemap;

void main() {
  vec3 color = texture(u_cubemap, P).rgb;
  O_color = vec4(tonemap(color), 1.0f);
}
