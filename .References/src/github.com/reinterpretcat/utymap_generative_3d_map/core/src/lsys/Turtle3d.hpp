#ifndef LSYS_TURTLES_TURTLE3D_HPP_DEFINED
#define LSYS_TURTLES_TURTLE3D_HPP_DEFINED

#include "lsys/Turtle.hpp"
#include "math/Vector3.hpp"
#include "math/Quaternion.hpp"

#include <stack>

namespace utymap {
namespace lsys {

/// Defines turtle which executes commands in 3d space.
class Turtle3d : public utymap::lsys::Turtle {
 public:

  Turtle3d();

  void jumpForward() override;

  void turnLeft() override;

  void turnRight() override;

  void turnAround() override;

  void pitchUp() override;

  void pitchDown() override;

  void rollLeft() override;

  void rollRight() override;

  void increment() override;

  void decrement() override;

  void scaleUp() override;

  void scaleDown() override;

  void switchStyle() override;

  void save() override;

  void restore() override;

  virtual void run(const LSystem &lsystem) override;

 protected:
  /// Defines turtle state.
  struct State {
    /// Specifies current length.
    double length = 0;
    /// Specifies current width (radius).
    double width = 0;
    /// Specifies current selected texture index.
    std::size_t texture = 0;

    utymap::math::Vector3 position = utymap::math::Vector3::zero();
    utymap::math::Vector3 direction = utymap::math::Vector3::up();
    utymap::math::Vector3 right = utymap::math::Vector3::right();
  };

  State state_;

 private:
  /// Creates rotation quaternion from current state.
  utymap::math::Quaternion createRotation(double angle) const;

  /// Angle defined by lsystem.
  double angle_ = 0;
  /// Scale factor defined by lsystem.
  double scale_ = 1;
  /// Stack for storing states.
  std::stack<State> states_;
};

}
}

#endif // LSYS_TURTLES_TURTLE3D_HPP_DEFINED
