const char* g_view3d_frag = R"(
#version 450

in vec3 P;
out vec4 color;

layout(std430, binding = 0) buffer vertexNormals {
  float N[];
};

void main(void) {
  int id = gl_PrimitiveID * 3;
  vec3 n = vec3(N[id], N[id + 1], N[id + 2]);
  color = vec4(n * 0.5f + 0.5f, 1.0f);
}
)";
