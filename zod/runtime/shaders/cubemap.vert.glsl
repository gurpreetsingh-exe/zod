layout(location = 0) in vec3 position;
out vec3 P;

layout(std140) uniform Camera {
  mat4 view_projection;
  vec4 direction;
};

void main() {
  P = position;
  mat4 mat = view_projection;
  mat[3] = vec4(0.0, 0.0, 0.0, 0.1);
  vec4 pos = mat * vec4(position, 1.0);
  gl_Position = pos.xyww;
}
