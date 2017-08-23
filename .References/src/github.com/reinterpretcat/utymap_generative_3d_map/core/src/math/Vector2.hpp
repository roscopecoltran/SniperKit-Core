#ifndef MATH_VECTOR2_HPP_DEFINED
#define MATH_VECTOR2_HPP_DEFINED

#include <cmath>
#include <limits>

namespace utymap {
namespace math {

/// Represents primitive which can be used as point or direction in 2d space.
struct Vector2 {
  double x;
  double y;
  Vector2() : x(0), y(0) {}
  Vector2(double x, double y) : x(x), y(y) {}

  bool operator==(const Vector2 &rhs) const {
    return ::std::fabs(x - rhs.x) < std::numeric_limits<double>::epsilon() &&
        ::std::fabs(y - rhs.y) < std::numeric_limits<double>::epsilon();
  }

  bool operator!=(const Vector2 &rhs) const {
    return !(*this==rhs);
  }

  Vector2 operator*(double scale) const {
    return Vector2(x*scale, y*scale);
  }

  Vector2 operator/(double scale) const {
    return Vector2(x/scale, y/scale);
  }

  Vector2 operator+(const Vector2 &rhs) const {
    return Vector2(x + rhs.x, y + rhs.y);
  }

  Vector2 operator-(const Vector2 &rhs) const {
    return Vector2(x - rhs.x, y - rhs.y);
  }

  Vector2 &operator+=(const Vector2 &rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  double magnitude() const {
    return ::std::sqrt(x*x + y*y);
  }

  /// Calculates dot product
  double dot(const Vector2 &v) const {
    return x*v.x + y*v.y;
  }

  Vector2 normalized() const {
    double length = magnitude();
    return length > std::numeric_limits<double>::epsilon()
           ? Vector2(x/length, y/length)
           : Vector2(); // degenerative case
  }

  static double distance(const Vector2 &v1, const Vector2 &v2) {
    double dx = v1.x - v2.x;
    double dy = v1.y - v2.y;
    return std::sqrt(dx*dx + dy*dy);
  }
};

}
}
#endif //PRIMITIVES_VECTOR2_HPP_DEFINED
