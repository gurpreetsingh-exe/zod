layout(location = 0) in vec2 position;
out vec2 uv;
out uint instance;

layout(std140, binding = 1) uniform Camera {
  mat4 view_projection;
  vec4 direction;
};

struct Node {
  vec2 offset;
  uint id;
  uint extra;
};

layout(std430, binding = 1) buffer Nodes { Node nodes[]; };

void main(void) {
  uv = position;
  gl_Position = view_projection * vec4(position * vec2(200.0f, 200.0f) +
                                           nodes[gl_InstanceID].offset,
                                       0.0f, 1.f);
  instance = gl_InstanceID;
}
