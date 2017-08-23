#include "builders/terrain/TerraExtras.hpp"
#include "entities/Area.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::lsys;
using namespace utymap::tests;

namespace {
const ColorGradient gradient = ColorGradient();
const TextureRegion textureRegion = TextureRegion();
const std::string stylesheetStr = "area|z16[amenity=forest] {"
    "lsys: tree;"
    "lsys-size: 1m;"
    "lsys-colors: gray,yellow;"
    "lsys-texture-indices: 0,0;"
    "lsys-texture-types: background,grass;"
    "lsys-texture-scales: 50,200;"
    "}";

StyleSheet createStyleSheet() {
  auto stylesheet = utymap::mapcss::MapCssParser().parse(stylesheetStr);
  LSystem lsystem;
  lsystem.axiom.push_back(std::make_shared<MoveForwardRule>());
  stylesheet.lsystems.emplace("tree", lsystem);
  return stylesheet;
}

struct Builders_Terrain_TerraExtrasFixture {
  Builders_Terrain_TerraExtrasFixture() :
      quadKey(16, 35203, 21493),
      dependencyProvider(),
      builderContext(
          quadKey,
          *dependencyProvider.getStyleProvider(createStyleSheet()),
          *dependencyProvider.getStringTable(),
          *dependencyProvider.getElevationProvider(),
          std::bind(&Builders_Terrain_TerraExtrasFixture::verifyMesh, this, std::placeholders::_1),
          nullptr,
          dependencyProvider.getCancellationToken()),
      isVerified(false) {
  }

  std::shared_ptr<Mesh> generateMesh() {
    auto mesh = std::make_shared<Mesh>("area");
    Polygon polygon(4, 0);
    polygon.addContour(std::vector<Vector2> {{0, 0}, {10, 0}, {10, 10}, {0, 10}});
    MeshBuilder builder(quadKey, *dependencyProvider.getElevationProvider());
    builder.addPolygon(*mesh, polygon,
                       MeshBuilder::GeometryOptions(5, 0, 0, 0),
                       MeshBuilder::AppearanceOptions(gradient, 0, 0, textureRegion, 0));
    return mesh;
  }

  Style generateStyle() {
    Area area = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), 0,
                                                  {{"amenity", "forest"}},
                                                  {{0, 0}, {10, 0}, {10, 10}, {0, 10}});
    return dependencyProvider.getStyleProvider(createStyleSheet())->forElement(area, 16);
  }

  void verifyMesh(const Mesh &mesh) {
    BOOST_CHECK_GT(mesh.vertices.size(), 0);
    BOOST_CHECK_GT(mesh.triangles.size(), 0);
    BOOST_CHECK_GT(mesh.colors.size(), 0);

    BOOST_CHECK_EQUAL(*std::max_element(mesh.triangles.begin(), mesh.triangles.end()),
                      static_cast<int>(mesh.vertices.size() - 1)/3);
    BOOST_CHECK_EQUAL(mesh.vertices.size()/3, mesh.colors.size());

    isVerified = true;
  }

  QuadKey quadKey;
  DependencyProvider dependencyProvider;
  BuilderContext builderContext;
  bool isVerified;
};
}

BOOST_FIXTURE_TEST_SUITE(Builders_Terrain_TerraExtras, Builders_Terrain_TerraExtrasFixture)

BOOST_AUTO_TEST_CASE(GivenMesh_WhenAddForest_ThenTreesAreAdded) {
  auto mesh = generateMesh();
  auto style = generateStyle();
  TerraExtras::Context extrasContext(*mesh, style);
  extrasContext.startVertex = 0, extrasContext.startTriangle = 0, extrasContext.startColor = 0;

  TerraExtras::addForest(builderContext, extrasContext);

  BOOST_CHECK(isVerified);
}

BOOST_AUTO_TEST_SUITE_END()
