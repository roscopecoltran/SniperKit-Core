#ifndef TESTS_LSYS_STRINGTURTLE_HPP_DEFINED
#define TESTS_LSYS_STRINGTURTLE_HPP_DEFINED

#include "lsys/Turtle.hpp"

namespace utymap {
namespace tests {

/// Defines turtle which writes its actions using common lsystem grammar notation.
class StringTurtle final : public utymap::lsys::Turtle {
 public:
  void moveForward() override { path += "F"; }

  void jumpForward() override { path += "G"; }

  void turnLeft() override { path += "+"; }

  void turnRight() override { path += "-"; }

  void turnAround() override { path += "|"; }

  void pitchUp() override { path += "^"; }

  void pitchDown() override { path += "&"; }

  void rollLeft() override { path += "\\"; }

  void rollRight() override { path += "/"; }

  void increment() override { path += "#"; }

  void decrement() override { path += "!"; }

  void scaleUp() override { path += ">"; }

  void scaleDown() override { path += "<"; }

  void switchStyle() override { path += "@"; }

  void save() override { path += "["; }

  void restore() override { path += "]"; }

  void say(const std::string &word) override { path += word; }

  std::string path;
};

}
}

#endif // TESTS_LSYS_STRINGTURTLE_HPP_DEFINED
