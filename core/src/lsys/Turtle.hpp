#ifndef LSYS_TURTLE_HPP_DEFINED
#define LSYS_TURTLE_HPP_DEFINED

#include <functional>

namespace utymap {
namespace lsys {

struct LSystem;

/// Defines abstract turtle behaviour by utilizing common l-system notation.
class Turtle {
 public:
  /// F: Move forward by line length drawing a line.
  virtual void moveForward() {}

  /// G: Move forward by line length without drawing a line.
  virtual void jumpForward() {}

  /// +: Turn left by turning angle.
  virtual void turnLeft() {}

  /// -: Turn right by turning angle.
  virtual void turnRight() {}

  /// |: Turn around (ie: turn by 180 degrees).
  virtual void turnAround() {}

  /// ^: Pitch up by turning angle.
  virtual void pitchUp() {}

  /// &: Pitch down by turning angle.
  virtual void pitchDown() {}

  /// \: Roll left by turning angle.
  virtual void rollLeft() {}

  /// /: Roll right by turning angle.
  virtual void rollRight() {}

  /// $: Increment the line width by line width scale factor.
  virtual void increment() {}

  /// !: Decrement the line width by line width scale factor.
  virtual void decrement() {}

  /// >: Multiply the line length by the line length scale factor.
  virtual void scaleUp() {}

  /// <: Divide the line length by the line length scale factor.
  virtual void scaleDown() {}

  /// @: Switch to next style.
  virtual void switchStyle() {}

  /// [: Saves current state on stack.
  virtual void save() {}

  /// ]: Restores current state from stack.
  virtual void restore() {}

  /// Performs instructions specific for given word.
  virtual void say(const std::string &word) {}

  virtual ~Turtle() {}

  /// Runs turtle using lsystem provided.
  virtual void run(const LSystem &lsystem);
};

}
}

#endif // LSYS_TURTLE_HPP_DEFINED
