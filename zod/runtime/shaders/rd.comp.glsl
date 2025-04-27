layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 2) readonly buffer vertexPositionIn {
  float in_position[];
};

layout(std140) uniform Camera {
  mat4 view_projection;
  vec4 direction;
};

layout(rgba8) uniform image2D u_texture;

uniform uint u_width;
uniform uint u_height;

const vec3 points[3] = {
  vec3(0, 0, 0),
  vec3(0, 1, 0),
  vec3(1, 1, 0),
};

void main(void) {
  vec2 uv = vec2(gl_GlobalInvocationID.xy) / vec2(u_width, u_height);
  // if (gl_GlobalInvocationID.x >= u_width * u_height) {
  //     return;
  // }
  // uint x = gl_GlobalInvocationID.x % u_width;
  // uint y = gl_GlobalInvocationID.x / u_height;
  // imageStore(u_texture, ivec2(x, y), vec4(float(x) / float(u_width), float(y)
  // / float(u_height), 1, 1));
  imageStore(u_texture, ivec2(gl_GlobalInvocationID.xy), vec4(uv, 0, 1));
}
