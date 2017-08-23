#ifndef MATH_QUATERNION_HPP_DEFINED
#define MATH_QUATERNION_HPP_DEFINED

#include "math/Vector3.hpp"

#include <cmath>

namespace utymap {
namespace math {

// Represents quaternion.
struct Quaternion final {
  double x, y, z, w;

  /// Creates quaternion from scalar and normalized vector.
  Quaternion(const Vector3 &v, double w) :
      Quaternion(v.x, v.y, v.z, w) {
  }

  /// Creates quaternion from four scalars.
  Quaternion(double x, double y, double z, double w) :
      x(x), y(y), z(z), w(w) {
  }

  /// Rotates quaternion.
  void rotate(const Quaternion &q) {
    x = w*q.x + x*q.w + y*q.z - z*q.y;
    y = w*q.y + y*q.w + z*q.x - x*q.z;
    z = w*q.z + z*q.w + x*q.y - y*q.x;
    w = w*q.w - x*q.x - y*q.y - z*q.z;
  }

  /// Multiples quanternions:
  // [ww' - dot(v, v'), wv' + w'v + cross(v, v')]
  Quaternion operator*(const Quaternion &q) const {
    return Quaternion(w*q.x + x*q.w + y*q.z - z*q.y,
                      w*q.y + y*q.w + z*q.x - x*q.z,
                      w*q.z + z*q.w + x*q.y - y*q.x,
                      w*q.w - x*q.x - y*q.y - z*q.z);
  }

  /// Multiples quaternion by given vector. Result is rotated vector by this quaternion.
  /// NOTE This implementation is different from reference one: p' = p * v * p_inverted.
  Vector3 operator*(const Vector3 &v) const {
    Quaternion q(v.x*w + v.z*y - v.y*z,
                 v.y*w + v.x*z - v.z*x,
                 v.z*w + v.y*x - v.x*y,
                 v.x*x + v.y*y + v.z*z);

    return Vector3(w*q.x + x*q.w + y*q.z - z*q.y,
                   w*q.y + y*q.w + z*q.x - x*q.z,
                   w*q.z + z*q.w + x*q.y - y*q.x)*(1./norm());
  }

  double norm() const {
    return x*x + y*y + z*z + w*w;
  }

  /// Returns inversed.
  Quaternion inversed() const {
    double in = 1/norm();
    return Quaternion(-x*in, -y*in, -z*in, w*in);
  }

  /// Creates indentity quaternion.
  static Quaternion identity() {
    return Quaternion(Vector3(0, 0, 0), 1);
  }

  /// Creates a unit quaternion rotating by axis angle representation.
  static Quaternion fromAngleAxis(double angle, const Vector3 &v) {
    double halfAngle = angle*0.5;
    return Quaternion(v*std::sin(halfAngle), std::cos(halfAngle));
  }

  /// Creates unit quaternion from euler angles in radians.
  static Quaternion fromEulerAngles(const Vector3 &v) {
    double t0 = std::cos(v.x*0.5);
    double t1 = std::sin(v.x*0.5);
    double t2 = std::cos(v.y*0.5);
    double t3 = std::sin(v.y*0.5);
    double t4 = std::cos(v.z*0.5);
    double t5 = std::sin(v.z*0.5);

    return Quaternion(t0*t3*t4 - t1*t2*t5,
                      t0*t2*t5 + t1*t3*t4,
                      t1*t2*t4 - t0*t3*t5,
                      t0*t2*t4 + t1*t3*t5);
  }

  /// Performs spherical linear interpolation.
  static Quaternion slerp(const Quaternion &q0, const Quaternion &q1, double t) {
    // compute dot product
    double cosOmega = q0.w*q1.w + q0.x*q1.x + q0.y*q1.y + q0.z*q1.z;

    // if negative, negate one of the input quaternions, to take the shorter 4D "arc"
    int sign = 1;
    if (cosOmega < 0) {
      sign = -sign;
      cosOmega = -cosOmega;
    }

    // check if they are very close together, to protect against divide-by-zero
    double k0, k1;
    if (cosOmega > 0.9999) {
      // very close - just use linear interpolation
      k0 = 1 - t;
      k1 = t;
    } else {
      // compute the sin of the angle using the trig identity sin^2(omega)+cos^2(omega) = 1
      double sinOmega = std::sqrt(1 - cosOmega*cosOmega);
      // compute the angle from its sine and cosine
      double omega = std::atan2(sinOmega, cosOmega);
      // compute inverse of denominator, so we only have to divide once
      double oneOverSinOmega = 1/sinOmega;
      // compute interpolation parameters
      k0 = std::sin((1 - t)*omega)*oneOverSinOmega*(sign > 0 ? 1 : -1);
      k1 = std::sin(t*omega)*oneOverSinOmega*(sign > 0 ? 1 : -1);
    }

    return Quaternion(q0.x*k0 + q1.x*k1,
                      q0.y*k0 + q1.y*k1,
                      q0.z*k0 + q1.z*k1,
                      q0.w*k0 + q1.w*k1);
  }
};

}
}
#endif //MATH_QUATERNION_HPP_DEFINED
