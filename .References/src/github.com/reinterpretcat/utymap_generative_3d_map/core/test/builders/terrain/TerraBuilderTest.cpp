#include "QuadKey.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/terrain/TerraBuilder.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::math;
using namespace utymap::tests;

namespace {
const std::string stylesheet =
    "canvas|z1 { grid-cell-size: 1%; ele-noise-freq: 0.05; color-noise-freq: 0.1; color:gradient(red); max-area: 5%;"
        "road-ele-noise-freq: 0; road-color-noise-freq: 0; road-color:gradient(red); road-max-area: 0;}"
        "area|z1[landuse=commercial] { builder: terrain; terrain-layer:road; }"

        "way|z1[highway][incline] { incline: eval(\"tag('incline')\"); }"
        "way|z1[highway] { builder: terrain; terrain-layer:road; width: 0.0000001; }"
        "way|z1[layer<0] { level: eval(\"tag('layer')\"); }";

struct Builders_Terrain_TerraBuilderFixture {
  DependencyProvider dependencyProvider;
  std::unique_ptr<BuilderContext> context = nullptr;

  std::unique_ptr<TerraBuilder> create(const QuadKey &quadKey,
                                       std::function<void(const utymap::math::Mesh &)> meshCallback) {
    context = utymap::utils::make_unique<BuilderContext>(quadKey,
                                                         *dependencyProvider.getStyleProvider(stylesheet),
                                                         *dependencyProvider.getStringTable(),
                                                         *dependencyProvider.getElevationProvider(),
                                                         meshCallback,
                                                         nullptr,
                                                         CancellationToken());
    return utymap::utils::make_unique<TerraBuilder>(*context);
  }
};
}

BOOST_FIXTURE_TEST_SUITE(Builders_Terrain_TerraBuilder, Builders_Terrain_TerraBuilderFixture)

BOOST_AUTO_TEST_CASE(GivenArea_WhenComplete_ThenSurfaceMeshIsNotEmpty) {
  bool isCalled = false;
  auto terraBuilder = create(QuadKey(1, 0, 0), [&](const Mesh &mesh) {
    if (mesh.name!="terrain_surface") return;
    BOOST_CHECK_GT(mesh.vertices.size(), 0);
    BOOST_CHECK_GT(mesh.triangles.size(), 0);
    isCalled = true;
  });
  ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), 0,
                                    {{"landuse", "commercial"}},
                                    {{0, 0}, {20, 0}, {20, 20}, {0, 20}})
      .accept(*terraBuilder);

  terraBuilder->complete();

  BOOST_CHECK(isCalled);
}

BOOST_AUTO_TEST_SUITE_END()
