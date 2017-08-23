#include "lsys/LSystemParser.hpp"

#include "lsys/StringTurtle.hpp"
#include <boost/test/unit_test.hpp>

#include "config.hpp"
#include <fstream>

using namespace utymap::lsys;
using namespace utymap::tests;

namespace {
const double ProbabilityPrecision = 0.01;
const std::string SimpleGrammar = "generations: 3\n"
    "angle: 45\n"
    "scale: 0.1\n"
    "axiom: D F ab G G F K\n"
    "F(1) -> F G\n"
    "G(0.8) -> F\n"
    "G(0.2) -> G G G G ab\n"
    "ab -> G G F ab";

std::shared_ptr<MoveForwardRule> move() { return std::make_shared<MoveForwardRule>(); }
std::shared_ptr<JumpForwardRule> jump() { return std::make_shared<JumpForwardRule>(); }
std::shared_ptr<WordRule> A() { return std::make_shared<WordRule>("ab"); }
}

BOOST_AUTO_TEST_SUITE(Lsys_LSystemParser)

BOOST_AUTO_TEST_CASE(GivenSimpleGrammar_WhenParse_ThenHasGenerations) {
  BOOST_CHECK_EQUAL(LSystemParser().parse(SimpleGrammar).generations, 3);
}

BOOST_AUTO_TEST_CASE(GivenSimpleGrammar_WhenParse_ThenHasAngle) {
  BOOST_CHECK_CLOSE(LSystemParser().parse(SimpleGrammar).angle, 45, ProbabilityPrecision);
}

BOOST_AUTO_TEST_CASE(GivenSimpleGrammar_WhenParse_ThenHasScale) {
  BOOST_CHECK_CLOSE(LSystemParser().parse(SimpleGrammar).scale, 0.1, ProbabilityPrecision);
}

BOOST_AUTO_TEST_CASE(GivenSimpleGrammar_WhenParse_ThenHasExpectedAmountOfAxiomRules) {
  BOOST_CHECK_EQUAL(LSystemParser().parse(SimpleGrammar).axiom.size(), 7);
}

BOOST_AUTO_TEST_CASE(GivenSimpleGrammar_WhenParse_ThenHasExpectedAmountOfProductions) {
  auto lsystem = LSystemParser().parse(SimpleGrammar);

  BOOST_CHECK_EQUAL(lsystem.productions.size(), 3);

  BOOST_CHECK_EQUAL(lsystem.productions[move()].size(), 1);
  BOOST_CHECK_CLOSE(lsystem.productions[move()].at(0).first, 1, ProbabilityPrecision);
  BOOST_CHECK_EQUAL(lsystem.productions[move()].at(0).second.size(), 2);

  BOOST_CHECK_EQUAL(lsystem.productions[jump()].size(), 2);
  BOOST_CHECK_CLOSE(lsystem.productions[jump()].at(0).first, 0.8, ProbabilityPrecision);
  BOOST_CHECK_EQUAL(lsystem.productions[jump()].at(0).second.size(), 1);
  BOOST_CHECK_CLOSE(lsystem.productions[jump()].at(1).first, 0.2, ProbabilityPrecision);
  BOOST_CHECK_EQUAL(lsystem.productions[jump()].at(1).second.size(), 5);

  BOOST_CHECK_EQUAL(lsystem.productions[A()].size(), 1);
  BOOST_CHECK_CLOSE(lsystem.productions[A()].at(0).first, 1, ProbabilityPrecision);
  BOOST_CHECK_EQUAL(lsystem.productions[A()].at(0).second.size(), 4);
}

BOOST_AUTO_TEST_CASE(GivenGrammarWithComment_WhenParse_ThenSkipsCommentLine) {
  const std::string grammar = "generations: 3\n"
      "#angle:35\n"
      "angle: 45\n"
      "#angle:55\n"
      "scale: 1.2\n"
      "axiom: A\n"
      "A -> G G";

  BOOST_CHECK_EQUAL(LSystemParser().parse(grammar).angle, 45);
}

BOOST_AUTO_TEST_CASE(GivenGrammarWithProbabilities_WhenParse_ThenStringTurtleBuildsProperPath) {
  StringTurtle turtle;
  const std::string grammar = "generations: 2\n"
      "angle: 45\n"
      "scale: 1.2\n"
      "axiom: A G\n"
      "G(0) -> G\n"
      "G(1) -> F F\n"
      "F -> G\n"
      "A -> G G";

  turtle.run(LSystemParser().parse(grammar));

  BOOST_CHECK_EQUAL(turtle.path, "FFFFGG");
}

BOOST_AUTO_TEST_CASE(GivenGrammarWithWords_WhenParse_ThenStringTurtleBuildsProperPath) {
  StringTurtle turtle;
  const std::string grammar = "generations: 1\n"
      "angle: 45\n"
      "scale: 1.2\n"
      "axiom: root F G leaf\n"
      "root(0) -> G\n"
      "root(1) -> F trunk\n"
      "G -> F\n"
      "leaf -> trunk G leaf";

  turtle.run(LSystemParser().parse(grammar));

  BOOST_CHECK_EQUAL(turtle.path, "FtrunkFFtrunkGleaf");
}

BOOST_AUTO_TEST_CASE(GivenGrammarInFile_WhenParse_ThenGrammarIsParsed) {
  StringTurtle turtle;
  std::ifstream file(TEST_MAPCSS_PATH "tree.lsys");

  turtle.run(LSystemParser().parse(file));

  BOOST_CHECK_EQUAL(turtle.path, "cylindersphere");
}

BOOST_AUTO_TEST_SUITE_END()
