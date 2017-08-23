#ifndef MATH_VECTOR3_HPP_DEFINED
#define MATH_VECTOR3_HPP_DEFINED

#include <cmath>
#include <limits>

namespace utymap {
namespace math {

/// Represents primitive which can be used as point or direction in 3d space.
struct Vector3 {
  double x;
  double y;
  double z;
  Vector3() : x(0), y(0), z(0) {}
  Vector3(double x, double y, double z) : x(x), y(y), z(z) {}

  bool operator==(const Vector3 &rhs) const {
    return std::fabs(x - rhs.x) < std::numeric_limits<double>::epsilon() &&
        std::fabs(y - rhs.y) < std::numeric_limits<double>::epsilon() &&
        std::fabs(z - rhs.z) < std::numeric_limits<double>::epsilon();
  }

  Vector3 operator*(double scale) const {
    return Vector3(x*scale, y*scale, z*scale);
  }

  Vector3 operator/(double scale) const {
    return Vector3(x/scale, y/scale, z/scale);
  }

  Vector3 operator+(const Vector3 &rhs) const {
    return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
  }

  Vector3 operator-(const Vector3 &rhs) const {
    return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
  }

  Vector3 &operator+=(const Vector3 &rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  double magnitude() const {
    return ::std::sqrt(x*x + y*y + z*z);
  }

  Vector3 normalized() const {
    double length = magnitude();
    return length > std::numeric_limits<double>::epsilon()
           ? Vector3(x/length, y/length, z/length)
           : Vector3(); // degenerative case
  }

  /// Creates vector which points up.
  static Vector3 up() {
    return Vector3(0, 1, 0);
  }

  /// Creates vector which points right.
  static Vector3 right() {
    return Vector3(1, 0, 0);
  }

  /// Creates vector with zero coordinates.
  static Vector3 zero() {
    return Vector3();
  }

  static double distance(const Vector3 &v1, const Vector3 &v2) {
    double dx = v1.x - v2.x;
    double dy = v1.y - v2.y;
    double dz = v1.z - v2.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
  }

  static double dot(const Vector3 &a, const Vector3 &b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
  }

  static Vector3 cross(const Vector3 &a, const Vector3 &b) {
    return Vector3(a.y*b.z - a.z*b.y,
                   a.z*b.x - a.x*b.z,
                   a.x*b.y - a.y*b.x);
  }
};

}
}
#endif //MATH_VECTOR3_HPP_DEFINED
