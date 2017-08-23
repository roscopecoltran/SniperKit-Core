#include "builders/generators/CylinderGenerator.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"
#include "builders/generators/LSystemGenerator.hpp"
#include "entities/Node.hpp"
#include "lsys/LSystemParser.hpp"

#include <boost/test/unit_test.hpp>
#include <fstream>

#include "config.hpp"
#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::index;
using namespace utymap::utils;
using namespace utymap::tests;

namespace {
const std::string stylesheet =
    "node|z1[natural=tree] {"
        "lsys: tree;"
        "lsys-size: 1m;"
        "lsys-colors: gray,yellow;"
        "lsys-texture-indices: 0,0;"
        "lsys-texture-types: background,grass;"
        "lsys-texture-scales: 50,200;"
        "}";

struct Builders_Generators_GeneratorFixture {
  Builders_Generators_GeneratorFixture() :
      dependencyProvider(),
      mesh(""),
      gradient(),
      textureRegion(),
      style(dependencyProvider.getStyleProvider(stylesheet)
                ->forElement(ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(),
                                                               0,
                                                               {{"natural", "tree"}}), 1)),
      builderContext(
          QuadKey(1, 35205, 21489),
          *dependencyProvider.getStyleProvider(stylesheet),
          *dependencyProvider.getStringTable(),
          *dependencyProvider.getElevationProvider(),
          [](const Mesh &) {},
          [](const Element &) {},
          dependencyProvider.getCancellationToken()),
      meshContext(mesh, style, gradient, textureRegion) {
  }

  DependencyProvider dependencyProvider;
  Mesh mesh;
  ColorGradient gradient;
  TextureRegion textureRegion;
  Style style;
  BuilderContext builderContext;
  MeshContext meshContext;
};
}

BOOST_FIXTURE_TEST_SUITE(Builders_Generators_Generator, Builders_Generators_GeneratorFixture)

BOOST_AUTO_TEST_CASE(GivenIcoSphereGeneratorWithSimpleData_WhenGenerate_ThenCanGenerateMesh) {
  IcoSphereGenerator(builderContext, meshContext)
      .setCenter(Vector3(0, 0, 0))
      .setSize(Vector3(10, 10, 10))
      .setRecursionLevel(2)
      .isSemiSphere(false)
      .setColorNoiseFreq(0.1)
      .setVertexNoiseFreq(0.5)
      .generate();

  BOOST_CHECK_GT(mesh.vertices.size(), 0);
  BOOST_CHECK_GT(mesh.triangles.size(), 0);
  BOOST_CHECK_GT(mesh.colors.size(), 0);
}

BOOST_AUTO_TEST_CASE(GivenCylinderGeneratorWithSimpleData_WhenGenerate_ThenCanGenerateMesh) {
  CylinderGenerator(builderContext, meshContext)
      .setCenter(Vector3(0, 0, 0))
      .setMaxSegmentHeight(5)
      .setRadialSegments(7)
      .setSize(Vector3(0.5, 10, 0.5))
      .setColorNoiseFreq(0.1)
      .generate();

  BOOST_CHECK_GT(mesh.vertices.size(), 0);
  BOOST_CHECK_GT(mesh.triangles.size(), 0);
  BOOST_CHECK_GT(mesh.colors.size(), 0);
}

BOOST_AUTO_TEST_CASE(GivenLSystemGeneratorWithSimpleData_WhenGenerate_ThenCanGenerateMesh) {
  GeoCoordinate center(52.53178, 13.38750);
  std::ifstream file(TEST_MAPCSS_PATH "tree.lsys");
  auto lsystem = utymap::lsys::LSystemParser().parse(file);

  LSystemGenerator(builderContext, style, mesh)
      .setPosition(center, 0)
      .run(lsystem);

  BOOST_CHECK_GT(mesh.vertices.size(), 0);
  BOOST_CHECK_GT(mesh.triangles.size(), 0);
  BOOST_CHECK_GT(mesh.colors.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
