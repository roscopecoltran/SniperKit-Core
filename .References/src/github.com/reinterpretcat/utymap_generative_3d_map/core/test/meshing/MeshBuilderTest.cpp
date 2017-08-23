#include "QuadKey.hpp"
#include "heightmap/FlatElevationProvider.hpp"
#include "builders/terrain/LineGridSplitter.hpp"
#include "mapcss/ColorGradient.hpp"
#include "builders/MeshBuilder.hpp"

#include <boost/test/unit_test.hpp>

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::math;

namespace {
typedef Vector2 DPoint;
struct Meshing_MeshingFixture {
  Meshing_MeshingFixture() :
      eleProvider(),
      builder(utymap::QuadKey(1, 1, 0), eleProvider),
      gradient(),
      textureRegion(),
      geometryOptions(0, 0, 0, 0),
      appearanceOptions(gradient, 0, 0, textureRegion, 0) {
  }

  FlatElevationProvider eleProvider;
  MeshBuilder builder;
  ColorGradient gradient;
  TextureRegion textureRegion;
  MeshBuilder::GeometryOptions geometryOptions;
  MeshBuilder::AppearanceOptions appearanceOptions;
};
}

BOOST_FIXTURE_TEST_SUITE(Meshing_MeshBuilder, Meshing_MeshingFixture)

BOOST_AUTO_TEST_CASE(GivenPolygon_WhenAddPolygon_ThenRefinesCorrectly) {
  Mesh mesh("");
  Polygon polygon(4, 0);
  geometryOptions.area = 5;
  polygon.addContour(std::vector<DPoint>
                         {
                             DPoint(0, 0),
                             DPoint(10, 0),
                             DPoint(10, 10),
                             DPoint(0, 10)
                         });

  builder.addPolygon(mesh, polygon, geometryOptions, appearanceOptions);

  BOOST_CHECK_EQUAL(mesh.vertices.size()/3, 23);
  BOOST_CHECK_EQUAL(mesh.triangles.size()/3, 34);
}

BOOST_AUTO_TEST_CASE(GivenPolygonWithHole_WhenAddPolygon_ThenRefinesCorrectly) {
  Mesh mesh("");
  Polygon polygon(8, 1);
  geometryOptions.area = 1;
  polygon.addContour(std::vector<DPoint>
                         {
                             DPoint(0, 0),
                             DPoint(10, 0),
                             DPoint(10, 10),
                             DPoint(0, 10)
                         });
  polygon.addHole(std::vector<DPoint>
                      {
                          DPoint(3, 3),
                          DPoint(6, 3),
                          DPoint(6, 6),
                          DPoint(3, 6)
                      });

  builder.addPolygon(mesh, polygon, geometryOptions, appearanceOptions);

  BOOST_CHECK(mesh.vertices.size() > 0);
  BOOST_CHECK(mesh.triangles.size() > 0);
}

BOOST_AUTO_TEST_CASE(GivenPolygonProcessedByGridSplitter_WhenAddPolygon_ThenRefinesCorrectly) {
  std::vector<Vector2> contour;
  LineGridSplitter splitter;
  int scale = 10;
  geometryOptions.area = 1./scale;
  splitter.setParams(scale, 1);
  std::vector<IntPoint> inputPoints
      {
          IntPoint(0*scale, 0*scale),
          IntPoint(10*scale, 0*scale),
          IntPoint(10*scale, 10*scale),
          IntPoint(0*scale, 10*scale)
      };
  std::size_t lastItemIndex = inputPoints.size() - 1;
  for (std::size_t i = 0; i <= lastItemIndex; i++) {
    IntPoint start = inputPoints[i];
    IntPoint end = inputPoints[i==lastItemIndex ? 0 : i + 1];
    splitter.split(start, end, contour);
  }

  Polygon polygon(contour.size(), 0);
  polygon.addContour(contour);

  Mesh mesh("");
  builder.addPolygon(mesh, polygon, geometryOptions, appearanceOptions);

  BOOST_CHECK(mesh.vertices.size() > 0);
  BOOST_CHECK(mesh.triangles.size() > 0);
}

BOOST_AUTO_TEST_CASE(GivenPolygonWithSharePoint_WhenAddPolygon_ThenRefinesCorrectly) {
  Polygon polygon(8, 0);
  polygon.addContour({{0, 0}, {10, 0}, {10, 10}, {0, 10}});
  polygon.addContour({{10, 10}, {15, 10}, {15, 15}, {10, 15}});
  Mesh mesh("");
  geometryOptions.area = 1;

  builder.addPolygon(mesh, polygon, geometryOptions, appearanceOptions);

  BOOST_CHECK(mesh.vertices.size() > 0);
}

BOOST_AUTO_TEST_CASE(GivenSimpleSquareWithTextureOptions_WhenAddPolygon_ThenTextureIsApplied) {

  Mesh mesh("");
  Polygon polygon(4, 0);
  polygon.addContour(std::vector<DPoint> {{0, 0}, {10, 0}, {10, 10}, {0, 10}});
  TextureRegion newTextureRegion(100, 100, 0, 0, 100, 100);
  MeshBuilder::AppearanceOptions newAppearanceOptions(gradient, 0, 0, newTextureRegion, 1);

  builder.addPolygon(mesh, polygon, geometryOptions, newAppearanceOptions);

  BOOST_CHECK(mesh.uvs.size() > 0);
  BOOST_CHECK_EQUAL(mesh.vertices.size()*2/3, mesh.uvs.size());
}

BOOST_AUTO_TEST_CASE(GivenSimpleSquareWithScale_WhenAddPolygon_ThenScaledTextureIsApplied) {
  Mesh mesh("");
  Polygon polygon(4, 0);
  MeshBuilder localBuilder(utymap::QuadKey(16, 35205, 21489), eleProvider);
  polygon
      .addContour(std::vector<DPoint> {{13.3874549, 52.530385}, {13.38747790, 52.53038981}, {13.38764498, 52.53042475},
                                       {13.38781206, 52.53045970}});
  TextureRegion newTextureRegion(1025, 1025, 513, 513, 512, 512);
  MeshBuilder::AppearanceOptions newAppearanceOptions(gradient, 0, 0, newTextureRegion, 100);

  localBuilder.addPolygon(mesh, polygon, geometryOptions, newAppearanceOptions);

  BOOST_CHECK(mesh.uvs.size() > 0);
  BOOST_CHECK_EQUAL(mesh.vertices.size()*2/3, mesh.uvs.size());
}

BOOST_AUTO_TEST_SUITE_END()
