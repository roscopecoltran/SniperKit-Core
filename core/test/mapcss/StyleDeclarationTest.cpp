#include "entities/Node.hpp"
#include "mapcss/StyleDeclaration.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::tests;

namespace {
struct MapCss_StyleDeclarationFixture {
  DependencyProvider dependencyProvider;
};
}

BOOST_FIXTURE_TEST_SUITE(MapCss_StyleDeclaration, MapCss_StyleDeclarationFixture)

BOOST_AUTO_TEST_CASE(GivenOnlySingleTag_WhenDoubleEvaluate_ThenReturnValue) {
  StyleDeclaration styleDeclaration(0, "eval(\"tag('height')\")");
  auto node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0, { { "height", "2.5" } });

  double result = styleDeclaration.evaluate<double>(node.tags, *dependencyProvider.getStringTable());

  BOOST_CHECK_EQUAL(result, 2.5);
}

BOOST_AUTO_TEST_CASE(GivenTwoTags_WhenDoubleEvaluate_ThenReturnValue) {
  StyleDeclaration styleDeclaration(0, "eval(\"tag('building:height') - tag('roof:height')\")");
  auto node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0,
                                                { { "building:height", "10" }, { "roof:height", "2.5" } });

  double result = styleDeclaration.evaluate<double>(node.tags, *dependencyProvider.getStringTable());

  BOOST_CHECK_EQUAL(result, 7.5);
}

BOOST_AUTO_TEST_CASE(GivenOneTagOneNumber_WhenDoubleEvaluate_ThenReturnValue) {
  StyleDeclaration styleDeclaration(0, "eval(\"tag('building:levels') * 3\")");
  auto node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0,
                                                { { "building:levels", "5" } });

  double result = styleDeclaration.evaluate<double>(node.tags, *dependencyProvider.getStringTable());

  BOOST_CHECK_EQUAL(result, 15);
}

BOOST_AUTO_TEST_CASE(GivenRawValue_WhenDoubleEvaluate_ThenThrowsException) {
  StyleDeclaration styleDeclaration(0, "13");
  auto node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0,
                                                { { "building:levels", "5" } });

  BOOST_CHECK_THROW(styleDeclaration.evaluate<double>(node.tags, *dependencyProvider.getStringTable()),
                    utymap::MapCssException);
}

BOOST_AUTO_TEST_CASE(GivenOneTagOneNumber_WhenStringEvaluate_ThenReturnValue) {
  StyleDeclaration styleDeclaration(0, "eval(\"tag('color')\")");
  auto node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0, 
                                                { { "color", "red" } });

  std::string result = styleDeclaration.evaluate<std::string>(node.tags, *dependencyProvider.getStringTable());

  BOOST_CHECK_EQUAL(result, "red");
}

BOOST_AUTO_TEST_CASE(GivenTagStringConcatenation_WhenStringEvaluate_ThenReturnValue) {
  StyleDeclaration styleDeclaration(0, "eval(\"tag('amenity') + '_string'\")");
  auto node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0,
                                                { { "amenity", "place_of_worship" } });

  std::string result = styleDeclaration.evaluate<std::string>(node.tags, *dependencyProvider.getStringTable());

  BOOST_CHECK_EQUAL(result, "place_of_worship_string");
}

BOOST_AUTO_TEST_CASE(GivenStringTagConcatenation_WhenStringEvaluate_ThenReturnValue) {
  StyleDeclaration styleDeclaration(0, "eval(\"'terrain_landuse_' + tag('landuse')\")");
  auto node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0,
  { { "landuse", "residential" } });

  std::string result = styleDeclaration.evaluate<std::string>(node.tags, *dependencyProvider.getStringTable());

  BOOST_CHECK_EQUAL(result, "terrain_landuse_residential");
}

BOOST_AUTO_TEST_CASE(GivenTwoTagsConcatenatedWithRawString_WhenStringEvaluate_ThenReturnValue) {
  StyleDeclaration styleDeclaration(0, "eval(\"tag('amenity') + '_' + tag('religion')\")");
  auto node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0,
  { { "amenity", "place_of_worship" }, {"religion", "christian"} });

  std::string result = styleDeclaration.evaluate<std::string>(node.tags, *dependencyProvider.getStringTable());

  BOOST_CHECK_EQUAL(result, "place_of_worship_christian");
}

BOOST_AUTO_TEST_SUITE_END()
