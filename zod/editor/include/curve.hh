#pragma once

namespace zod {

class BezierCurve {
public:
  BezierCurve(vec2 p1, vec2 p2, vec2 p3, vec2 p4)
      : m_control_points(p1, p2, p3, p4) {}

public:
  auto get(f32 t) -> vec2 {
    auto t1 = std::pow(1.0f - t, 3.0f);
    auto t2 = (1.0f - t) * (1.0f - t);
    return t1 * m_control_points[0] + 3 * t2 * t * m_control_points[1] +
           3 * (1.0f - t) * t * t * m_control_points[2] +
           t * t * t * m_control_points[3];
  }

private:
  vec2 m_control_points[4];
};

class RoundCurve {
public:
  RoundCurve(vec2 p1, vec2 p2, f32 rounding)
      : m_p1(p1), m_p2(p2), m_rounding(rounding) {}

public:
  auto get() -> Vector<vec2> {
    auto points = Vector<vec2>();
    auto midpoint = (m_p1 + m_p2) * 0.5f;
    points.push_back(m_p1);
    points.push_back(vec2(m_p1.x, midpoint.y - m_rounding));
    constexpr usize segments = 12;
    constexpr auto pi = std::numbers::pi_v<f32>;
    auto offset = vec2(m_p1.x, midpoint.y) - m_rounding;
    for (usize i = 0; i < segments; ++i) {
      auto t = f32(i) / f32(segments) * pi * 0.5f;
      vec2 point = offset + vec2(glm::cos(t), glm::sin(t)) * m_rounding;
      points.push_back(point);
    }
    points.push_back(vec2(m_p1.x - m_rounding, midpoint.y));
    points.push_back(vec2(m_p2.x + m_rounding, midpoint.y));
    offset = vec2(m_p2.x, midpoint.y) + m_rounding;
    for (usize i = 0; i < segments; ++i) {
      auto t = (f32(segments - i - 1) / f32(segments) * pi * 0.5f) + pi;
      vec2 point = offset + vec2(glm::cos(t), glm::sin(t)) * m_rounding;
      points.push_back(point);
    }
    points.push_back(vec2(m_p2.x, midpoint.y + m_rounding));
    points.push_back(m_p2);
    return points;
  }

private:
  vec2 m_p1;
  vec2 m_p2;
  f32 m_rounding;
};

} // namespace zod
