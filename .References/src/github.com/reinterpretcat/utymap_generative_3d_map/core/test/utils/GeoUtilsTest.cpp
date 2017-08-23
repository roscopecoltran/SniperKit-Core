#include "BoundingBox.hpp"
#include "QuadKey.hpp"
#include "utils/GeoUtils.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap;
using namespace utymap::utils;

namespace {
const double Precision = 0.1e-7;
const double TestLatitude = 52.53171;
const double TestLongitude = 13.38730;
}

BOOST_AUTO_TEST_SUITE(Utils_GeoUtils)

BOOST_AUTO_TEST_CASE(GivenTestLocationAtFirstLod_WhenGetQuadKey_ThenReturnValidQuadKey) {
  QuadKey quadKey = GeoUtils::GeoCoordinateToQuadKey(GeoCoordinate(TestLatitude, TestLongitude), 1);

  BOOST_CHECK_EQUAL(quadKey.levelOfDetail, 1);
  BOOST_CHECK_EQUAL(quadKey.tileX, 1);
  BOOST_CHECK_EQUAL(quadKey.tileY, 0);
}

BOOST_AUTO_TEST_CASE(GivenTestLocationAtNineLod_WhenGetQuadKey_ThenReturnValidQuadKey) {
  QuadKey quadKey = GeoUtils::GeoCoordinateToQuadKey(GeoCoordinate(TestLatitude, TestLongitude), 9);

  BOOST_CHECK_EQUAL(quadKey.levelOfDetail, 9);
  BOOST_CHECK_EQUAL(quadKey.tileX, 275);
  BOOST_CHECK_EQUAL(quadKey.tileY, 167);
}

BOOST_AUTO_TEST_CASE(GivenTestLocationAtNineteenLod_WhenGetQuadKey_ThenReturnValidQuadKey) {
  QuadKey quadKey = GeoUtils::GeoCoordinateToQuadKey(GeoCoordinate(TestLatitude, TestLongitude), 19);

  BOOST_CHECK_EQUAL(quadKey.levelOfDetail, 19);
  BOOST_CHECK_EQUAL(quadKey.tileX, 281640);
  BOOST_CHECK_EQUAL(quadKey.tileY, 171914);
}

BOOST_AUTO_TEST_CASE(GivenQuadKeyAtNineteenLod_WhenGetBoundgingBox_ThenReturnValidBoundingBox) {
  QuadKey quadKey(19, 281640, 171914);

  utymap::BoundingBox boundingBox = GeoUtils::quadKeyToBoundingBox(quadKey);

  BOOST_CHECK_CLOSE(boundingBox.minPoint.latitude, 52.531678559, Precision);
  BOOST_CHECK_CLOSE(boundingBox.maxPoint.latitude, 52.532096259, Precision);
  BOOST_CHECK_CLOSE(boundingBox.minPoint.longitude, 13.386840820, Precision);
  BOOST_CHECK_CLOSE(boundingBox.maxPoint.longitude, 13.387527465, Precision);
}

BOOST_AUTO_TEST_CASE(GivenQuadKeyAtNineteenLod_WhenToString_ThenReturnValidCode) {
  QuadKey quadKey(19, 281640, 171914);

  std::string code = GeoUtils::quadKeyToString(quadKey);

  BOOST_CHECK_EQUAL("1202102332220103020", code);
}

BOOST_AUTO_TEST_CASE(GivenBboxAtLodOne_WhenVisitTileRange_VisitsOneTile) {
  BoundingBox bbox(GeoCoordinate(1, 1), GeoCoordinate(2, 2));
  int count = 0;
  std::shared_ptr<QuadKey> lastQuadKey;

  GeoUtils::visitTileRange(bbox, 1, [&](const QuadKey &quadKey, const BoundingBox &) {
    count++;
    lastQuadKey = std::make_shared<QuadKey>(quadKey);
  });

  BOOST_CHECK_EQUAL(1, count);
  BOOST_CHECK_EQUAL(1, lastQuadKey->tileX);
  BOOST_CHECK_EQUAL(0, lastQuadKey->tileY);
}

BOOST_AUTO_TEST_CASE(GivenBboxAtLodOne_WhenVisitTileRange_VisitsTwoTiles) {
  BoundingBox bbox(GeoCoordinate(-1, 1), GeoCoordinate(2, 2));
  int count = 0;

  GeoUtils::visitTileRange(bbox, 1, [&](const QuadKey &quadKey, const BoundingBox &) {
    count++;
  });

  BOOST_CHECK_EQUAL(2, count);
}

BOOST_AUTO_TEST_SUITE_END()
