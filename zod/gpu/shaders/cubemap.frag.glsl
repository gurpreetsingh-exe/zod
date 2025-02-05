#version 450

in vec3 P;
out vec4 color;

uniform samplerCube u_cubemap;

void main() { color = texture(u_cubemap, P); }
