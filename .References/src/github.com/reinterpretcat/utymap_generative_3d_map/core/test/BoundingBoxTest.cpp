#include "BoundingBox.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap;

namespace {
const double Precision = 0.1e-7;
const BoundingBox bbox(GeoCoordinate(0, 0), GeoCoordinate(10, 10));
}

BOOST_AUTO_TEST_SUITE(BoundingBox_Suite)

BOOST_AUTO_TEST_CASE(GivenInnerBoundingBox_WhenContains_ThenReturnTrue) {
  const BoundingBox b(GeoCoordinate(2, 2), GeoCoordinate(4, 4));

  bool result = bbox.contains(b);

  BOOST_CHECK(result);
}

BOOST_AUTO_TEST_CASE(GivenOuterBoundingBox_WhenContains_ThenReturnFalse) {
  const BoundingBox b(GeoCoordinate(10, 10), GeoCoordinate(20, 20));

  bool result = bbox.contains(b);

  BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(GivenInnerCoordinate_WhenContains_ThenReturnTrue) {
  bool result = bbox.contains(GeoCoordinate(5, 5));

  BOOST_CHECK(result);
}

BOOST_AUTO_TEST_CASE(GivenOuterCoordinate_WhenContains_ThenReturnFalse) {
  bool result = bbox.contains(GeoCoordinate(15, 15));

  BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(GivenIntersectBoundingBox_WhenIntersects_ThenReturnTrue) {
  const BoundingBox b(GeoCoordinate(5, 5), GeoCoordinate(15, 15));

  bool result = bbox.intersects(b);

  BOOST_CHECK(result);
}

BOOST_AUTO_TEST_CASE(GivenOuterBoundingBox_WhenIntersects_ThenReturnFalse) {
  const BoundingBox b(GeoCoordinate(11, 11), GeoCoordinate(15, 15));

  bool result = bbox.intersects(b);

  BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(GivenBoundingBox_WhenExpandOperator_ThenReturnExpanded) {
  BoundingBox a = bbox;
  const BoundingBox b(GeoCoordinate(-10, -10), GeoCoordinate(15, 15));

  a += b;

  BOOST_CHECK_CLOSE(a.minPoint.latitude, -10, Precision);
  BOOST_CHECK_CLOSE(a.minPoint.longitude, -10, Precision);
  BOOST_CHECK_CLOSE(a.maxPoint.latitude, 15, Precision);
  BOOST_CHECK_CLOSE(a.maxPoint.longitude, 15, Precision);
}

BOOST_AUTO_TEST_SUITE_END()
