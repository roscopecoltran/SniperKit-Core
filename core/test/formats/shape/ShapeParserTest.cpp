#include "config.hpp"
#include "formats/shape/ShapeParser.hpp"
#include "formats/shape/CountableShapeDataVisitor.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::formats;
using namespace utymap::index;

namespace {
const double Precision = 0.1e-7;

struct Formats_Shape_ShapeParserFixture {
  ShapeParser<CountableShapeDataVisitor> parser;
  CountableShapeDataVisitor visitor;
};
}

BOOST_FIXTURE_TEST_SUITE(Formats_ShapeParser, Formats_Shape_ShapeParserFixture)

BOOST_AUTO_TEST_CASE(GivenTestPointFile_WhenParse_ThenVisitsAllRecords) {
  parser.parse(TEST_SHAPE_POINT_FILE, visitor);

  BOOST_CHECK_EQUAL(visitor.nodes, 4);
}

BOOST_AUTO_TEST_CASE(GivenTestPointFile_WhenParse_ThenHasCorrectCoordinate) {
  parser.parse(TEST_SHAPE_POINT_FILE, visitor);

  BOOST_CHECK_CLOSE(visitor.lastCoordinate.latitude, -0.552155778109874, Precision);
  BOOST_CHECK_CLOSE(visitor.lastCoordinate.longitude, 0.996334426278262, Precision);
}

BOOST_AUTO_TEST_CASE(GivenTestPointFile_WhenParse_ThenHasCorrectTags) {
  parser.parse(TEST_SHAPE_POINT_FILE, visitor);

  BOOST_CHECK_EQUAL(visitor.lastTags.size(), 1);
  BOOST_CHECK_EQUAL(visitor.lastTags[0].key, "test");
  BOOST_CHECK_EQUAL(visitor.lastTags[0].value, "test4");
}

BOOST_AUTO_TEST_CASE(GivenTestLineFile_WhenParse_ThenVisitsAllRecords) {
  parser.parse(TEST_SHAPE_LINE_FILE, visitor);

  BOOST_CHECK_EQUAL(visitor.ways, 1);
}

BOOST_AUTO_TEST_CASE(GivenTestLineFile_WhenParse_ThenHasCorrectTags) {
  parser.parse(TEST_SHAPE_LINE_FILE, visitor);

  BOOST_CHECK_EQUAL(visitor.lastTags.size(), 1);
  BOOST_CHECK_EQUAL(visitor.lastTags[0].key, "test");
  BOOST_CHECK_EQUAL(visitor.lastTags[0].value, "Foo");
}

BOOST_AUTO_TEST_CASE(GivenTestLineFile_WhenParse_ThenHasCorrectCoordinates) {
  parser.parse(TEST_SHAPE_LINE_FILE, visitor);

  BOOST_CHECK_EQUAL(visitor.lastCoordinates.size(), 6);
  BOOST_CHECK(!visitor.isRing);
  BOOST_CHECK_CLOSE(visitor.lastCoordinates[0].latitude, -0.488351110417178, Precision);
  BOOST_CHECK_CLOSE(visitor.lastCoordinates[0].longitude, -1.42333489468323, Precision);
  BOOST_CHECK_CLOSE(visitor.lastCoordinates[5].latitude, -0.576696034914758, Precision);
  BOOST_CHECK_CLOSE(visitor.lastCoordinates[5].longitude, -1.10921960758073, Precision);
}

BOOST_AUTO_TEST_CASE(GivenTestMultiPolyFile_WhenParse_ThenVisitsAllRecords) {
  parser.parse(TEST_SHAPE_MULTIPOLY_FILE, visitor);

  BOOST_CHECK_EQUAL(visitor.relations, 1);
}

BOOST_AUTO_TEST_CASE(GivenTestMultiPolyFile_WhenParse_ThenHasCorrectTags) {
  parser.parse(TEST_SHAPE_MULTIPOLY_FILE, visitor);

  BOOST_CHECK_EQUAL(visitor.lastTags.size(), 10);
  BOOST_CHECK_EQUAL(visitor.lastTags[0].key, "ComID");
  BOOST_CHECK_EQUAL(visitor.lastTags[0].value, "105569740");
}

BOOST_AUTO_TEST_CASE(GivenTestMultiPolyFile_WhenParse_ThenHasCorrectGeometry) {
  parser.parse(TEST_SHAPE_MULTIPOLY_FILE, visitor);

  BOOST_CHECK_EQUAL(visitor.lastMembers.size(), 2);
  BOOST_CHECK(visitor.lastMembers[0].isRing);
  BOOST_CHECK(visitor.lastMembers[1].isRing);
  BOOST_CHECK_EQUAL(visitor.lastMembers[0].coordinates.size(), 35);
  BOOST_CHECK_EQUAL(visitor.lastMembers[1].coordinates.size(), 10);
  BOOST_CHECK_CLOSE(visitor.lastMembers[0].coordinates[0].latitude, 47.7065701259499, Precision);
  BOOST_CHECK_CLOSE(visitor.lastMembers[0].coordinates[0].longitude, -94.9833954296734, Precision);
  BOOST_CHECK_CLOSE(visitor.lastMembers[1].coordinates[0].latitude, 47.7061475259505, Precision);
  BOOST_CHECK_CLOSE(visitor.lastMembers[1].coordinates[0].longitude, -94.9856752963366, Precision);
}

BOOST_AUTO_TEST_SUITE_END()
