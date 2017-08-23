#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "mapcss/Style.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::utils;
using namespace utymap::tests;

namespace {
const std::string stylesheet = "way|z16[meters] { width: 10m; }"
  "way|z16[percent] { width: 10%; }"
  "way|z16[water] { width: -1m; }"
  "node|z16[place] { builder: place; }"
  "node|z16[place=stop] { builder: stop; }"
  "node|z16[amenity] { builder: amenity; }"
  "node|z16[amenity=bar] { builder: amenity; }";
struct MapCss_StyleFixture {
  DependencyProvider dependencyProvider;
  BoundingBox boundingBox = GeoUtils::quadKeyToBoundingBox(QuadKey(16, 35205, 21489));
};
}

BOOST_FIXTURE_TEST_SUITE(MapCss_Style, MapCss_StyleFixture)

BOOST_AUTO_TEST_CASE(GivenValueInMeters_WhenGetValue_ThenReturnPositiveValue) {
  int lod = 16;
  Way way = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(),
                                             0, {std::make_pair("meters", "")}, {{52.52975, 13.38810}});
  Style style = dependencyProvider.getStyleProvider(stylesheet)->forElement(way, lod);

  double width = style.getValue("width", boundingBox);

  BOOST_CHECK(width > 0);
}

BOOST_AUTO_TEST_CASE(GivenValueInPercents_WhenGetValue_ThenReturnPositiveValue) {
  int lod = 16;
  Way way = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(),
                                             0, {std::make_pair("percent", "")}, {{52.52975, 13.38810}});
  Style style = dependencyProvider.getStyleProvider(stylesheet)->forElement(way, lod);

  double width = style.getValue("width", boundingBox);

  BOOST_CHECK(width > 0);
}

BOOST_AUTO_TEST_CASE(GivenNegativeValueInMeters_WhenGetValue_ThenReturnExpectedValue) {
  int lod = 16;
  Way way = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(),
                                             0, {std::make_pair("water", "")}, {{52.52975, 13.38810}});
  Style style = dependencyProvider.getStyleProvider(stylesheet)->forElement(way, lod);

  double width = style.getValue("width", 1);

  BOOST_CHECK_EQUAL(width, -1);
}

BOOST_AUTO_TEST_CASE(GivenBuilderDeclarations_WhenGetBuilders_ThenReturnsAll) {
  int lod = 16;
  Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(),
    0, { std::make_pair("place", "stop") });
  Style style = dependencyProvider.getStyleProvider(stylesheet)->forElement(node, lod);

  auto builders = style.getBuilders();

  BOOST_CHECK_EQUAL(builders.size(), 2);
  BOOST_CHECK_EQUAL(builders.at(0), "place");
  BOOST_CHECK_EQUAL(builders.at(1), "stop");
}

BOOST_AUTO_TEST_CASE(GivenBuilderDeclarations_WhenGetBuilders_ThenReturnsUnique) {
  int lod = 16;
  Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(),
    0, { std::make_pair("amenity", "bar") });
  Style style = dependencyProvider.getStyleProvider(stylesheet)->forElement(node, lod);

  auto builders = style.getBuilders();

  BOOST_CHECK_EQUAL(builders.size(), 1);
  BOOST_CHECK_EQUAL(builders.at(0), "amenity");
}

BOOST_AUTO_TEST_SUITE_END()
