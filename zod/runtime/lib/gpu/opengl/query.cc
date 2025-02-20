#include "./query.hh"

namespace zod {

GLQuery::GLQuery() { glCreateQueries(GL_TIMESTAMP, 2, m_id); }

auto GLQuery::begin() -> void { glQueryCounter(m_id[0], GL_TIMESTAMP); }

auto GLQuery::end() -> void { glQueryCounter(m_id[1], GL_TIMESTAMP); }

auto GLQuery::get_time() -> f32 {
  GLint stop = 0;
  while (not stop) {
    glGetQueryObjectiv(m_id[1], GL_QUERY_RESULT_AVAILABLE, &stop);
  }

  glGetQueryObjecti64v(m_id[0], GL_QUERY_RESULT, &m_start);
  glGetQueryObjecti64v(m_id[1], GL_QUERY_RESULT, &m_end);
  return f32(m_end - m_start) / 1000000.0f;
}

} // namespace zod
