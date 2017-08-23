#include "utils/NoiseUtils.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::utils;

namespace {
const double Tolerance = 1e-3;
}

BOOST_AUTO_TEST_SUITE(Utils_NoiseUtils)

BOOST_AUTO_TEST_CASE(GivenTestValues_WhenPerlin3d_ThenReturnExpectedValues) {
  // Compare with original C# implementation
  BOOST_CHECK_CLOSE(NoiseUtils::perlin3D(1, 1, 1, 0.1), 0.1862334, Tolerance);
  BOOST_CHECK_CLOSE(NoiseUtils::perlin3D(10, 42, 10, 0.15), 0.4103059, Tolerance);
  BOOST_CHECK_CLOSE(NoiseUtils::perlin3D(52, 120, 13, 0.12), -0.1014592, Tolerance);
}

BOOST_AUTO_TEST_SUITE_END()
