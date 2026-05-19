struct PointDrawData {
  vec2 position;
  vec2 uv;
};

layout(std430, binding = 1) readonly buffer GPUDrawData {
  PointDrawData g_draw_data[];
};

out vec2 v_texcoords;

uniform mat4 u_projection;

void main(void) {
  vec2 position = g_draw_data[gl_VertexID].position;
  v_texcoords = g_draw_data[gl_VertexID].uv;
  gl_Position = u_projection * vec4(position, 0.f, 1.f);
}
