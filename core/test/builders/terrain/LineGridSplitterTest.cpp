#include "builders/terrain/LineGridSplitter.hpp"

#include <boost/test/unit_test.hpp>

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::math;

namespace {
typedef std::vector<Vector2> DoublePoints;
typedef std::vector<IntPoint> IntPoints;
const double Precision = 0.1e-9;
}

BOOST_AUTO_TEST_SUITE(Builders_Terrain_LineGridSplitter)

BOOST_AUTO_TEST_CASE(GivenHorizontal_WhenSplitWithIntStep_ThenCanSplit) {
  IntPoint start(0, 0);
  IntPoint end(10, 0);
  LineGridSplitter splitter;
  DoublePoints result;

  splitter.split(start, end, result);

  for (int i = 0; i <= 10; ++i) {
    BOOST_CHECK_CLOSE(i, result[i].x, Precision);
    BOOST_CHECK_CLOSE(0, result[i].y, Precision);
  }
}

BOOST_AUTO_TEST_CASE(GivenVertical_WhenSplitWithIntStep_ThenCanSplit) {
  IntPoint start(0, 0);
  IntPoint end(0, 10);
  LineGridSplitter splitter;
  DoublePoints result;

  splitter.split(start, end, result);

  for (int i = 0; i <= 10; ++i) {
    BOOST_CHECK_CLOSE(0, result[i].x, Precision);
    BOOST_CHECK_CLOSE(i, result[i].y, Precision);
  }
}

BOOST_AUTO_TEST_CASE(Given45Angle_WhenSplitWithIntStep_ThenCanSplit) {
  IntPoint start(0, 0);
  IntPoint end(-10, 10);
  LineGridSplitter splitter;
  DoublePoints result;

  splitter.split(start, end, result);

  for (int i = 0; i <= 10; ++i) {
    BOOST_CHECK_CLOSE(-i, result[i].x, Precision);
    BOOST_CHECK_CLOSE(i, result[i].y, Precision);
  }
}

BOOST_AUTO_TEST_CASE(Given45Angle_WhenSplitWithHighLoD_ThenCanSplit) {
  LineGridSplitter splitter;
  IntPoint start(0, 0);
  IntPoint end(10, 10);
  DoublePoints result;

  splitter.split(start, end, result);

  for (int i = 0; i <= 10; ++i) {
    BOOST_CHECK_CLOSE(i, result[i].x, Precision);
    BOOST_CHECK_CLOSE(i, result[i].y, Precision);
  }
}

// These tests are for some bugs observed for real data
BOOST_AUTO_TEST_CASE(GivenSpecificCase1_WhenSplit_ThenCanSplit) {
  LineGridSplitter splitter;
  splitter.setParams(1E8, 3);
  IntPoint start(5286462881, -916628251);
  IntPoint end(5388136261, -968852454);
  DoublePoints result;

  splitter.split(start, end, result);

  for (int i = 0; i < result.size(); ++i) {
    BOOST_CHECK_LT(std::abs(result[i].x), 54);
    BOOST_CHECK_LT(std::abs(result[i].y), 10);
  }
}

// These tests are for some bugs observed for real data
BOOST_AUTO_TEST_CASE(GivenSpecificCase2_WhenSplit_ThenCanSplit) {
  LineGridSplitter splitter;
  splitter.setParams(1E7, 3);
  IntPoint start(-428193799, 626823300);
  IntPoint end(-411886999, 634824599);
  DoublePoints result;

  splitter.split(start, end, result);

  BOOST_CHECK_EQUAL(result.size(), 4);
}

BOOST_AUTO_TEST_CASE(GivenSpecificCase3_WhenSplit_ThenDoNotInflateDuplicates) {
  LineGridSplitter splitter;
  splitter.setParams(10000000, 0.0006103515625);
  IntPoints points = {
      {133691881, 525218163},
      {133693424, 525219786},
      {133692010, 525218310}
  };
  DoublePoints result;

  auto lastItemIndex = points.size() - 1;
  for (int i = 0; i <= lastItemIndex; i++)
    splitter.split(points[i], points[i==lastItemIndex ? 0 : i + 1], result);

  // TODO better to check duplicates directly
  BOOST_CHECK_EQUAL(result.size(), 6);
}

BOOST_AUTO_TEST_SUITE_END()
