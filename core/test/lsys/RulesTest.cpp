#include "lsys/LSystem.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::lsys;

BOOST_AUTO_TEST_SUITE(Lsys_Rules)

BOOST_AUTO_TEST_CASE(GivenTwoWordRulesWithDifferentWords_Productions_HandlesThemDifferently) {
  auto lsystem = LSystem();

  lsystem.productions[std::make_shared<WordRule>("1")].push_back(std::make_pair<double, LSystem::Rules>(
      1, {std::make_shared<WordRule>("1")}));
  lsystem.productions[std::make_shared<WordRule>("2")].push_back(std::make_pair<double, LSystem::Rules>(
      1, {std::make_shared<WordRule>("2")}));

  BOOST_CHECK_EQUAL(
      static_cast<const WordRule &>(*lsystem.productions[std::make_shared<WordRule>("1")][0].second[0]).word, "1");
  BOOST_CHECK_EQUAL(
      static_cast<const WordRule &>(*lsystem.productions[std::make_shared<WordRule>("2")][0].second[0]).word, "2");
}

BOOST_AUTO_TEST_SUITE_END()
