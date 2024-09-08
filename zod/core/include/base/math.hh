#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>

#include "macros.hh"

FMT(glm::vec2, "{{ {}, {} }}", v.x, v.y);
FMT(glm::vec3, "{{ {}, {}, {} }}", v.x, v.y, v.z);
FMT(glm::vec4, "{{ {}, {}, {}, {} }}", v.x, v.y, v.z, v.w);
FMT(glm::mat3, "{{ {},\n  {},\n  {} }}", v[0], v[1], v[2]);
FMT(glm::mat4, "{{ {},\n  {},\n  {},\n  {} }}", v[0], v[1], v[2], v[3]);

namespace zod {

using glm::cross;
using glm::dot;
using glm::inverse;
using glm::lookAt;
using glm::mat3;
using glm::mat4;
using glm::normalize;
using glm::ortho;
using glm::perspective;
using glm::quat;
using glm::radians;
using glm::rotate;
using glm::scale;
using glm::translate;
using glm::vec2;
using glm::vec3;
using glm::vec4;

#define ADDROF(v) glm::value_ptr(v)

} // namespace zod
