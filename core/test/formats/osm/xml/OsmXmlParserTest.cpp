#include "BoundingBox.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "formats/osm/xml/OsmXmlParser.hpp"

#include "config.hpp"
#include "test_utils/ElementUtils.hpp"
#include "test_utils/DependencyProvider.hpp"

#include <fstream>
#include <boost/test/unit_test.hpp>
#include <numeric>

using namespace utymap::entities;
using namespace utymap::formats;

namespace {
struct Formats_Osm_Xml_OsmXmlParserFixture {
  utymap::tests::DependencyProvider dependencyProvider;

  utymap::entities::Tag createTag(const std::string &key, const std::string &value) {
    return utymap::tests::ElementUtils::createTag(*dependencyProvider.getStringTable(), key, value);
  }
};

template<typename Iterator>
bool reduce(Iterator begin, Iterator end) {
  return std::accumulate(begin, end, true, [](bool a, bool b) {
    return a && b;
  });
}

void assertTags(const std::vector<utymap::entities::Tag> &expectedTags,
                const std::vector<utymap::entities::Tag> &actualTags) {
  BOOST_CHECK_EQUAL(expectedTags.size(), actualTags.size());
  for (int i = 0; i < actualTags.size(); ++i) {
    BOOST_CHECK_EQUAL(expectedTags.at(i).key, actualTags.at(i).key);
    BOOST_CHECK_EQUAL(expectedTags.at(i).value, actualTags.at(i).value);
  }
}

void assertGeometry(const std::vector<utymap::GeoCoordinate> &expectedGeometry,
                    const std::vector<utymap::GeoCoordinate> &actualGeometry) {
  BOOST_CHECK_EQUAL(expectedGeometry.size(), actualGeometry.size());
  for (int i = 0; i < actualGeometry.size(); ++i) {
    BOOST_CHECK_EQUAL(expectedGeometry.at(i).latitude, actualGeometry.at(i).latitude);
    BOOST_CHECK_EQUAL(expectedGeometry.at(i).longitude, actualGeometry.at(i).longitude);
  }
}

void assertNode(const Node &node,
                const utymap::GeoCoordinate &coordinate,
                const std::vector<utymap::entities::Tag> &tags) {
  assertGeometry({coordinate}, {node.coordinate});
  assertTags(tags, node.tags);
}

void assertWay(const Way &way,
               const std::vector<utymap::GeoCoordinate> &geometry,
               const std::vector<utymap::entities::Tag> &tags) {
  assertGeometry(geometry, way.coordinates);
  assertTags(tags, way.tags);
}
}

BOOST_FIXTURE_TEST_SUITE(Formats_Osm_Xml_Parser, Formats_Osm_Xml_OsmXmlParserFixture)

BOOST_AUTO_TEST_CASE(GivenDefaultOsmXml_WhenParserParse_ThenHasExpectedElementCount) {
  std::ifstream istream(TEST_XML_FILE, std::ios::in);
  OsmXmlParser<CountableOsmDataVisitor> parser;
  CountableOsmDataVisitor visitor;

  parser.parse(istream, visitor);

  BOOST_CHECK_EQUAL(1, visitor.bounds);
  BOOST_CHECK_EQUAL(7653, visitor.nodes);
  BOOST_CHECK_EQUAL(1116, visitor.ways);
  BOOST_CHECK_EQUAL(92, visitor.relations);
}

BOOST_AUTO_TEST_CASE(GivenDummyOverpassXml_WhenParserParse_ThenHasExpectedElementCount) {
  std::ifstream istream(TEST_OVERPASS_DUMMY_FILE, std::ios::in);
  OsmXmlParser<CountableOsmDataVisitor> parser;
  CountableOsmDataVisitor visitor;

  parser.parse(istream, visitor);

  BOOST_CHECK_EQUAL(0, visitor.bounds);
  BOOST_CHECK_EQUAL(2, visitor.nodes);
  BOOST_CHECK_EQUAL(1, visitor.ways);
  BOOST_CHECK_EQUAL(1, visitor.relations);
}

BOOST_AUTO_TEST_CASE(GivenTestDummyOsmXml_WhenParserParse_ThenHasProperNodes) {
  std::vector<bool> checkList{false, false};
  std::ifstream istream(TEST_OSM_DUMMY_FILE, std::ios::in);
  OsmXmlParser<OsmDataVisitor> parser;
  OsmDataVisitor visitor(*dependencyProvider.getStringTable(), [&](Element &element) {
    if (Node *node = dynamic_cast<Node *>(&element)) {
      if (node->id==42421951) {
        assertNode(*node,
                   utymap::GeoCoordinate(40.7033242, -74.0077787),
                   {createTag("highway", "traffic_signals")});
        checkList[0] = true;
      } else if (node->id==42424605) {
        assertNode(*node, utymap::GeoCoordinate(40.8142100, -73.9341897), {});
        checkList[1] = true;
      }
    }
    return true;
  });

  parser.parse(istream, visitor);
  visitor.complete();

  BOOST_CHECK(reduce(checkList.begin(), checkList.end()));
}

BOOST_AUTO_TEST_CASE(GivenTestDummyOsmXml_WhenParserParse_ThenHasProperWay) {
  std::vector<bool> checkList{false};
  std::ifstream istream(TEST_OSM_DUMMY_FILE, std::ios::in);
  OsmXmlParser<OsmDataVisitor> parser;
  OsmDataVisitor visitor(*dependencyProvider.getStringTable(), [&](Element &element) {
    if (Way *way = dynamic_cast<Way *>(&element)) {
      if (way->id==32934353) {
        assertWay(*way,
                  {
                      utymap::GeoCoordinate(40.7033242, -74.0077787),
                      utymap::GeoCoordinate(40.8142100, -73.9341897)
                  },
                  {
                      // NOTE eating spaces is known issue
                      createTag("alt_name", "FranklinDelanoRooseveltDrive"),
                      createTag("lanes", "3"),
                      createTag("tiger:reviewed", "no")
                  });
        checkList[0] = true;
      }
    }
    return true;
  });

  parser.parse(istream, visitor);
  visitor.complete();

  BOOST_CHECK(reduce(checkList.begin(), checkList.end()));
}

BOOST_AUTO_TEST_SUITE_END()
