#pragma once

#include "widget.hh"

namespace zod {

// Function to generate vertices for a round square
// std::vector<float> generateRoundSquareVertices(float L, float R,
//                                                int numSegments) {
//   std::vector<float> vertices;

//   float halfL = L / 2.0f;
//   float cornerAngleStep = M_PI / 2.0f / numSegments;

//   // Top-right corner (quarter circle)
//   for (int i = 0; i <= numSegments; ++i) {
//     float theta = i * cornerAngleStep;
//     float x = halfL - R + R * cos(theta);
//     float y = halfL - R + R * sin(theta);
//     vertices.push_back(x);
//     vertices.push_back(y);
//   }

//   // Top edge
//   vertices.push_back(halfL - R);
//   vertices.push_back(halfL);
//   vertices.push_back(-halfL + R);
//   vertices.push_back(halfL);

//   // Top-left corner (quarter circle)
//   for (int i = 0; i <= numSegments; ++i) {
//     float theta = M_PI / 2.0 + i * cornerAngleStep;
//     float x = -halfL + R + R * cos(theta);
//     float y = halfL - R + R * sin(theta);
//     vertices.push_back(x);
//     vertices.push_back(y);
//   }

//   // Left edge
//   vertices.push_back(-halfL);
//   vertices.push_back(halfL - R);
//   vertices.push_back(-halfL);
//   vertices.push_back(-halfL + R);

//   // Bottom-left corner (quarter circle)
//   for (int i = 0; i <= numSegments; ++i) {
//     float theta = M_PI + i * cornerAngleStep;
//     float x = -halfL + R + R * cos(theta);
//     float y = -halfL + R + R * sin(theta);
//     vertices.push_back(x);
//     vertices.push_back(y);
//   }

//   // Bottom edge
//   vertices.push_back(-halfL + R);
//   vertices.push_back(-halfL);
//   vertices.push_back(halfL - R);
//   vertices.push_back(-halfL);

//   // Bottom-right corner (quarter circle)
//   for (int i = 0; i <= numSegments; ++i) {
//     float theta = 3 * M_PI / 2.0 + i * cornerAngleStep;
//     float x = halfL - R + R * cos(theta);
//     float y = -halfL + R + R * sin(theta);
//     vertices.push_back(x);
//     vertices.push_back(y);
//   }

//   // Right edge
//   vertices.push_back(halfL);
//   vertices.push_back(-halfL + R);
//   vertices.push_back(halfL);
//   vertices.push_back(halfL - R);

//   return vertices;
// }

template <typename T>
class IRect {
public:
  using value_type = vec<2, T>;

  IRect(value_type position, value_type size, vec4 color)
      : m_position(position), m_size(size), m_color(color) {}
  IRect(value_type position, value_type size, value_type pivot_point,
        vec4 color)
      : m_position(position), m_size(size), m_pivot_point(pivot_point),
        m_color(color) {}

public:
  auto bottom_left() -> value_type { return m_position; }
  auto bottom_right() -> value_type {
    return value_type(m_position.x + m_size.x, m_position.y);
  }
  auto top_left() -> value_type {
    return value_type(m_position.x, m_position.y + m_size.y);
  }
  auto top_right() -> value_type { return m_position + m_size; }

  auto draw(Geometry& g) -> void {
    auto start = g.points.size();

    g.points.push_back(top_left());
    g.points.push_back(top_right());
    g.points.push_back(bottom_left());
    g.points.push_back(bottom_right());

    g.indices.push_back(start + 1);
    g.indices.push_back(start + 2);
    g.indices.push_back(start);
    g.indices.push_back(start + 2);
    g.indices.push_back(start + 3);
    g.indices.push_back(start + 1);

    for (usize i = 0; i < 4; ++i) { g.colors.push_back(m_color); }
  }

private:
  vec<2, T> m_position;
  vec<2, T> m_size;
  vec<2, T> m_pivot_point = {};
  vec4 m_color;
};

using Rect = IRect<f32>;

} // namespace zod
