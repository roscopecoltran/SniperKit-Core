#include "lsys/Turtle3d.hpp"
#include "lsys/LSystem.hpp"
#include "utils/MathUtils.hpp"

using namespace utymap::lsys;
using namespace utymap::math;

Turtle3d::Turtle3d() : states_() {
}

void Turtle3d::jumpForward() {
  state_.position = state_.position + state_.direction*state_.length;
}

void Turtle3d::turnLeft() {
  auto rotation = createRotation(-angle_);
  state_.direction = rotation*state_.direction;
  state_.right = rotation*state_.right;
}

void Turtle3d::turnRight() {
  auto rotation = createRotation(angle_);
  state_.direction = rotation*state_.direction;
  state_.right = rotation*state_.right;
}

void Turtle3d::turnAround() {
  auto rotation = createRotation(utymap::utils::deg2Rad(180));
  state_.direction = rotation*state_.direction;
  state_.right = rotation*state_.right;
}

void Turtle3d::pitchUp() {
  state_.direction = createRotation(angle_)*state_.direction;
}

void Turtle3d::pitchDown() {
  state_.direction = createRotation(-angle_)*state_.direction;
}

void Turtle3d::rollLeft() {
  state_.right = createRotation(-angle_)*state_.right;
}

void Turtle3d::rollRight() {
  state_.right = createRotation(angle_)*state_.right;
}

void Turtle3d::increment() {
  state_.width *= scale_;
}

void Turtle3d::decrement() {
  state_.width /= scale_;
}

void Turtle3d::scaleUp() {
  state_.length *= scale_;
}

void Turtle3d::scaleDown() {
  state_.length /= scale_;
}

void Turtle3d::switchStyle() {
  ++state_.texture;
}

void Turtle3d::save() {
  states_.push(state_);
}

void Turtle3d::restore() {
  state_ = states_.top();
  states_.pop();
}

void Turtle3d::run(const LSystem &lsystem) {
  angle_ = utymap::utils::deg2Rad(lsystem.angle);
  scale_ = lsystem.scale;

  Turtle::run(lsystem);
}

utymap::math::Quaternion Turtle3d::createRotation(double angle) const {
  return Quaternion::fromAngleAxis(angle, Vector3::cross(state_.direction, state_.right));
}
