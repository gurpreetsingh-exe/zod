const char* g_transform_comp = R"(
#version 450

layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 2) readonly buffer vertexPositionIn {
  float in_position[];
};

layout(std430, binding = 3) buffer vertexPositionOut {
  float out_position[];
};

uniform uint u_num_vertices;
uniform vec3 u_offset;

void main(void) {
  if (gl_GlobalInvocationID.x >= u_num_vertices) {
    return;
  }
  uint id = gl_GlobalInvocationID.x * 3;
  out_position[id] = in_position[id] + u_offset.x;
  out_position[id + 1] = in_position[id + 1] + u_offset.y;
  out_position[id + 2] = in_position[id + 2] + u_offset.z;
}
)";
