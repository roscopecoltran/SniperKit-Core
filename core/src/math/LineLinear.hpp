#ifndef MATH_LINELINEAR_HPP_DEFINED
#define MATH_LINELINEAR_HPP_DEFINED

#include "math/Vector2.hpp"

namespace utymap {
namespace math {

/// Geometry line in linear form. General form:
/// Ax + By + C = 0;
/// <see href="http://en.wikipedia.org/wiki/Linear_equation"/>
struct LineLinear final {
  double A;
  double B;
  double C;

  /// Linear line from two points on line.
  LineLinear(const Vector2 &p1, const Vector2 &p2) {
    A = p1.y - p2.y;
    B = p2.x - p1.x;
    C = p1.x*p2.y - p2.x*p1.y;
  }

  /// Linear line represented by coefficients.
  LineLinear(double pA, double pB, double pC) {
    A = pA;
    B = pB;
    C = pC;
  }

  /// Gets collision point of two lines.
  Vector2 collide(const LineLinear &line) const {
    return collide(*this, line);
  }

  /// <summary> Collision point of two lines. </summary>
  static Vector2 collide(const LineLinear &line1, const LineLinear &line2) {
    return collide(line1.A, line1.B, line1.C, line2.A, line2.B, line2.C);
  }

  /// <summary> Collision point of two lines. </summary>
  static Vector2 collide(double A1, double B1, double C1, double A2, double B2, double C2) {
    double WAB = A1*B2 - A2*B1;
    double WBC = B1*C2 - B2*C1;
    double WCA = C1*A2 - C2*A1;

    return WAB==0 ? Vector2() : Vector2(WBC/WAB, WCA/WAB);
  }

  /// <summary> Check whether point belongs to line. </summary>
  bool contains(const Vector2 &point) const {
    return std::abs((point.x*A + point.y*B + C)) < 1E-8;
  }
};

}
}
#endif //MATH_LINELINEAR_HPP_DEFINED
