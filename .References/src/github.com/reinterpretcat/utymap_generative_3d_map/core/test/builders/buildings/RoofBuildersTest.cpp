#include "GeoCoordinate.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/buildings/roofs/DomeRoofBuilder.hpp"
#include "builders/buildings/roofs/MansardRoofBuilder.hpp"
#include "builders/buildings/roofs/SkillionRoofBuilder.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::tests;

namespace {
const std::string stylesheet = "area|z16[building=yes] { roof-color:gradient(red); }";
struct Builders_Buildings_Roofs_RoofBuildersFixture {
  DependencyProvider dependencyProvider;

  std::shared_ptr<BuilderContext> builderContext;
  std::shared_ptr<MeshContext> meshContext;
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Style> style;
  ColorGradient gradient;
  TextureRegion textureRegion;

  template<typename T>
  T createRoofBuilder() {
    builderContext = dependencyProvider.createBuilderContext(QuadKey(16, 1, 0), stylesheet);
    Area building =
        ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), 0, {{"building", "yes"}}, {});
    style = std::make_shared<Style>(dependencyProvider.getStyleProvider(stylesheet)->forElement(building, 16));
    mesh = std::make_shared<Mesh>("");
    gradient = ColorGradient();
    meshContext = std::make_shared<MeshContext>(*mesh, *style, gradient, textureRegion);

    return T(*builderContext, *meshContext);
  }
};
}

BOOST_FIXTURE_TEST_SUITE(Builders_Buildings_Roofs_RoofBuilders, Builders_Buildings_Roofs_RoofBuildersFixture)

BOOST_AUTO_TEST_CASE(GivenDome_WhenBuilds_ThenMeshIsBuilt) {
  auto builder = createRoofBuilder<DomeRoofBuilder>();
  builder.setMinHeight(10);
  builder.setColorNoiseFreq(0);
  Polygon polygon(0, 0);
  polygon.addContour({{0, 0}, {0, 10}, {10, 10}, {10, 0}});

  builder.build(polygon);

  BOOST_CHECK_GT(mesh->vertices.size(), 0);
  BOOST_CHECK_GT(mesh->triangles.size(), 0);
  BOOST_CHECK_GT(mesh->colors.size(), 0);
}

BOOST_AUTO_TEST_CASE(GivenMansard_WhenBuilds_ThenMeshIsBuilt) {
  auto builder = createRoofBuilder<MansardRoofBuilder>();
  builder.setMinHeight(10);
  builder.setHeight(5);
  builder.setColorNoiseFreq(0);
  Polygon polygon(0, 0);
  polygon.addContour({{13.3873453, 52.5316191}, {13.3871987, 52.5317429}, {13.3876304, 52.5317804},
                      {13.3876814, 52.5316476}});

  builder.build(polygon);

  BOOST_CHECK_GT(mesh->vertices.size(), 0);
  BOOST_CHECK_GT(mesh->triangles.size(), 0);
  BOOST_CHECK_GT(mesh->colors.size(), 0);
}

BOOST_AUTO_TEST_CASE(GivenSkillionWithSlope_WhenBuilds_ThenMeshIsBuilt) {
  auto builder = createRoofBuilder<SkillionRoofBuilder>();
  Polygon polygon(0, 0);
  polygon.addContour({{37.618875, 55.7535279}, {37.6187893, 55.7534727}, {37.6190606, 55.7534625},
                      {37.6190376, 55.7535286}});
  builder.setMinHeight(30);
  builder.setHeight(5);
  builder.setDirection("311.2");

  builder.build(polygon);

  BOOST_CHECK_GT(mesh->vertices.size(), 0);
  BOOST_CHECK_GT(mesh->triangles.size(), 0);
  BOOST_CHECK_GT(mesh->colors.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
