#include "heightmap/GridElevationProvider.hpp"

#include "config.hpp"
#include <boost/test/unit_test.hpp>

using namespace utymap;
using namespace utymap::heightmap;

namespace {

// Elevation data for given quadkey is:
//  3  4  5
//  0  1  2
// -3 -2 -1
struct Heightmap_GridElevationProviderFixture {
  Heightmap_GridElevationProviderFixture() :
      quadKey(16, 35205, 21489),
      bbox(utymap::utils::GeoUtils::quadKeyToBoundingBox(quadKey)),
      eleProvider(TEST_ASSETS_PATH "index/") {
  }

  QuadKey quadKey;
  BoundingBox bbox;
  GridElevationProvider eleProvider;
};

const double Precision = 0.1;
}

BOOST_FIXTURE_TEST_SUITE(Heightmap_GridElevationProvider, Heightmap_GridElevationProviderFixture)

BOOST_AUTO_TEST_CASE(GivenLocationAtBottomLeft_WhenGetElevation_ThenReturnExpectedNegativeHeight) {
  double ele = eleProvider.getElevation(quadKey, bbox.minPoint);

  BOOST_CHECK_CLOSE(ele, -3, Precision);
}

BOOST_AUTO_TEST_CASE(GivenLocationAtTopRight_WhenGetElevation_ThenReturnExpectedPositiveHeight) {
  double ele = eleProvider.getElevation(quadKey, bbox.maxPoint);

  BOOST_CHECK_CLOSE(ele, 5, Precision);
}

BOOST_AUTO_TEST_CASE(GivenLocationInCenter_WhenGetElevation_ThenReturnExpectedHeight) {
  double ele = eleProvider.getElevation(quadKey, bbox.center());

  BOOST_CHECK_CLOSE(ele, 1, Precision);
}

BOOST_AUTO_TEST_CASE(GivenLocationInTheMiddleFromCenterToTopRight_WhenGetElevation_ThenReturnExpectedInterpolatedHeight) {
  GeoCoordinate coordinate(bbox.center().latitude + bbox.height()/4,
                           bbox.center().longitude + bbox.width()/4);

  double ele = eleProvider.getElevation(quadKey, coordinate);

  BOOST_CHECK_CLOSE(ele, 3, Precision);
}

BOOST_AUTO_TEST_CASE(GivenLocationNearTopRight_WhenGetElevation_ThenReturnExpectedInterpolatedHeight) {
  GeoCoordinate coordinate(bbox.maxPoint.latitude,
                           bbox.center().longitude + bbox.width()/4);

  double ele = eleProvider.getElevation(quadKey, coordinate);

  BOOST_CHECK_CLOSE(ele, 4.5, Precision);
}

BOOST_AUTO_TEST_SUITE_END()
