#include "config.hpp"
#include "ExportLib.cpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/ElementUtils.hpp"

using namespace utymap::entities;
using namespace utymap::utils;

namespace {
const char *InMemoryStoreKey = "InMemory";
const char *NaturalEarthMapcss = TEST_MAPCSS_PATH "natural_earth.z1.mapcss";

// Use global variable as it is used inside lambda which is passed as function.
bool isCalled;

struct ExportLibFixture {
  ExportLibFixture() {
    ::configure(TEST_ASSETS_PATH, [](const char *message) {
      BOOST_FAIL(message);
    });
    ::registerInMemoryStore(InMemoryStoreKey);
  }

  void loadQuadKeys(int levelOfDetails, int startX, int endX, int startY, int endY,
                    const char *mapcss = TEST_MAPCSS_DEFAULT) const {
    isCalled = false;
    utymap::CancellationToken cancelToken;
    for (int i = startX; i <= endX; ++i) {
      for (int j = startY; j <= endY; ++j) {
        ::loadQuadKey(0, mapcss, i, j, levelOfDetails, 0,
                      [](int tag,
                         const char *name,
                         const double *vertices, int vertexCount,
                         const int *triangles, int triCount,
                         const int *colors, int colorCount,
                         const double *uvs, int uvCount,
                         const int *uvMap, int uvMapCount) {
                        isCalled = true;
                        BOOST_CHECK_GT(vertexCount, 0);
                        BOOST_CHECK_GT(triCount, 0);
                        BOOST_CHECK_GT(colorCount, 0);
                        // NOTE ignore uvs as it is optional
                      },
                      [](int tag, uint64_t id, const char **tags, int size, const double *vertices,
                         int vertexCount, const char **style, int styleSize) {
                        isCalled = true;
                      },
                      [](const char *message) {
                        BOOST_FAIL(message);
                      }, &cancelToken);
      }
    }
    BOOST_CHECK(isCalled);
  }

  static void callback(const char *msg) { BOOST_CHECK(msg==nullptr); }

  ~ExportLibFixture() {
    ::cleanup();
    std::remove((std::string(TEST_ASSETS_PATH) + "string.idx").c_str());
    std::remove((std::string(TEST_ASSETS_PATH) + "string.dat").c_str());
  }
};
}

BOOST_FIXTURE_TEST_SUITE(ExportLib, ExportLibFixture)

BOOST_AUTO_TEST_CASE(GivenNaturalEarthTestData_WhenAllQuadKeysAreLoadedAtZoomOne_ThenCallbacksAreCalled) {
  ::addToStoreInRange(InMemoryStoreKey, NaturalEarthMapcss, TEST_SHAPE_NE_110M_LAND, 1, 1, callback);
  ::addToStoreInRange(InMemoryStoreKey, NaturalEarthMapcss, TEST_SHAPE_NE_110M_RIVERS, 1, 1, callback);
  ::addToStoreInRange(InMemoryStoreKey, NaturalEarthMapcss, TEST_SHAPE_NE_110M_LAKES, 1, 1, callback);

  // This data increases execution time. Also causes some issues.
  //::addToStoreInRange(naturalEarthMapcss, TEST_SHAPE_NE_110M_ADMIN, 1, 1, callback);
  //::addToStoreInRange(naturalEarthMapcss, TEST_SHAPE_NE_110M_BORDERS, 1, 1, callback);

  ::addToStoreInRange(InMemoryStoreKey, NaturalEarthMapcss, TEST_SHAPE_NE_110M_POPULATED_PLACES, 1, 1, callback);

  loadQuadKeys(1, 0, 1, 0, 1, NaturalEarthMapcss);
}

BOOST_AUTO_TEST_CASE(GivenTestData_WhenQuadKeysAreLoadedAtBirdEyeZoomLevel_ThenCallbacksAreCalled) {
  ::addToStoreInQuadKey(InMemoryStoreKey, TEST_MAPCSS_DEFAULT, TEST_JSON_2_FILE, 8800, 5373, 14, callback);

  loadQuadKeys(14, 8800, 8800, 5373, 5373);
}

/// This case tests storing lod range.
BOOST_AUTO_TEST_CASE(GivenTestData_WhenDataIsLoadedInLodRangeAtDetailedZoom_ThenCallbacksAreCalled) {
  ::addToStoreInRange(InMemoryStoreKey, TEST_MAPCSS_DEFAULT, TEST_XML_FILE, 16, 16, callback);

  loadQuadKeys(16, 35205, 35205, 21489, 21489);
}

/// This case tests dynamic addition incremental addition/search to store.
BOOST_AUTO_TEST_CASE(GivenTestData_WhenQuadKeysAreLoadedInSequenceAtDetailedZoom_ThenCallbacksAreCalled) {
  ::addToStoreInQuadKey(InMemoryStoreKey, TEST_MAPCSS_DEFAULT, TEST_XML_FILE, 35205, 21489, 16, callback);
  loadQuadKeys(16, 35205, 35205, 21489, 21489);

  ::addToStoreInQuadKey(InMemoryStoreKey, TEST_MAPCSS_DEFAULT, TEST_XML_FILE, 35204, 21490, 16, callback);
  loadQuadKeys(16, 35204, 35204, 21490, 21490);
}

BOOST_AUTO_TEST_CASE(GivenTestData_WhenQuadKeyIsLoaded_ThenHasDataReturnsTrue) {
  ::addToStoreInQuadKey(InMemoryStoreKey, TEST_MAPCSS_DEFAULT, TEST_XML_FILE, 35205, 21489, 16, callback);

  BOOST_CHECK(::hasData(35205, 21489, 16));
}

BOOST_AUTO_TEST_CASE(GivenTestData_WhenSpecificQuadKeyIsLoaded_ThenHasDataReturnsFalseForAnother) {
  ::addToStoreInQuadKey(InMemoryStoreKey, TEST_MAPCSS_DEFAULT, TEST_XML_FILE, 35205, 21489, 16, callback);

  BOOST_CHECK(!::hasData(35204, 21489, 16));
}

BOOST_AUTO_TEST_CASE(GivenElement_WhenAddInMemory_ThenItIsAdded) {
  const std::vector<double> vertices = {5, 5, 20, 5, 20, 10, 5, 10, 5, 5};
  const std::vector<const char *> tags = {"featurecla", "Lake", "scalerank", "0"};

  ::addToStoreElement(InMemoryStoreKey, NaturalEarthMapcss, 1, vertices.data(), 10,
                      const_cast<const char **>(tags.data()), 4, 1, 1, callback);

  BOOST_CHECK(::hasData(1, 0, 1));
}

BOOST_AUTO_TEST_SUITE_END()
