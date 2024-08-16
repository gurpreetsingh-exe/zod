const char* g_node_vert = R"(
#version 450

layout (location = 0) in vec2 position;
out vec2 uv;

layout (std140, binding = 1) uniform Camera {
    mat4 view_projection;
    vec4 direction;
};

layout (std430, binding = 1) buffer nodePosition {
  vec2 offset[];
};

void main(void) {
  uv = position;
  gl_Position = view_projection * vec4(position * vec2(84.0f, 100.0f) + offset[gl_InstanceID], 0.0f, 1.f);
}
)";
