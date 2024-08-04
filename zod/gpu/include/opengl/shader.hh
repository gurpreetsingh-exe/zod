#include <glad/glad.h>

#include "../shader.hh"

namespace zod {

class GLShader : public GPUShader {
public:
  GLShader(std::string /* name */);

public:
  auto init_vertex_shader(const char* /* source */) -> void override;
  auto init_fragment_shader(const char* /* source */) -> void override;
  auto compile() -> void override;
  auto bind() -> void override;
  auto unbind() -> void override;
  auto uniform_int(const std::string&, i32) -> void override;

private:
  auto create_shader(GLuint /* type */, const char* /* source */) -> GLuint;

private:
  GLuint m_id;
  GLuint m_vert;
  GLuint m_frag;
};

} // namespace zod
