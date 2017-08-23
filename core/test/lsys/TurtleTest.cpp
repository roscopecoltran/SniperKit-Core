#include "lsys/LSystem.hpp"

#include "lsys/StringTurtle.hpp"
#include <boost/test/unit_test.hpp>

using namespace utymap::lsys;
using namespace utymap::tests;

namespace {

std::shared_ptr<MoveForwardRule> move() { return std::make_shared<MoveForwardRule>(); }
std::shared_ptr<JumpForwardRule> jump() { return std::make_shared<JumpForwardRule>(); }
}

BOOST_AUTO_TEST_SUITE(Lsys_Turtle)

BOOST_AUTO_TEST_CASE(GivenSimpleLSystemWithOneGeneration_Build_PerformsExpectedCommands) {
  StringTurtle turtle;
  LSystem lsystem;
  lsystem.generations = 1;
  lsystem.angle = 1;
  lsystem.axiom.push_back(move());
  lsystem.productions[move()].push_back(std::make_pair<double, LSystem::Rules>(1, {move(), jump(), move()}));

  turtle.run(lsystem);

  BOOST_CHECK_EQUAL(turtle.path, "FGF");
}

BOOST_AUTO_TEST_CASE(GivenSimpleLSystemWithThreeGeneration_Build_PerformsExpectedCommands) {
  StringTurtle turtle;
  LSystem lsystem;
  lsystem.generations = 3;
  lsystem.angle = 1;
  lsystem.axiom.push_back(move());
  lsystem.productions[move()].push_back(std::make_pair<double, LSystem::Rules>(1, {jump(), move(), jump(), jump()}));

  turtle.run(lsystem);

  BOOST_CHECK_EQUAL(turtle.path, "GGGFGGGGGG");
}

BOOST_AUTO_TEST_SUITE_END()
