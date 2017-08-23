#include "heightmap/SrtmElevationProvider.hpp"

#include "config.hpp"
#include <boost/test/unit_test.hpp>

using namespace utymap;
using namespace utymap::heightmap;

BOOST_AUTO_TEST_SUITE(Heightmap_SrtmElevationProvider)

BOOST_AUTO_TEST_CASE(GivenTestLocation_WhenGetElevation_ThenReturnExpectedInteger) {
  SrtmElevationProvider eleProvider(TEST_ASSETS_PATH "index/");

  double ele = eleProvider.getElevation(QuadKey(16, 35205, 21489), 52.5317429, 13.3871987);

  BOOST_CHECK_CLOSE(ele, 34.853, 0.01);
}

BOOST_AUTO_TEST_SUITE_END()
