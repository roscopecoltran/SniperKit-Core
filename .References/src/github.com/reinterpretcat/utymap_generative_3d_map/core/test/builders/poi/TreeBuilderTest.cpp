#include "QuadKey.hpp"
#include "builders/poi/TreeBuilder.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Relation.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::lsys;
using namespace utymap::tests;

namespace {
const std::string stylesheetStr =
    "node|z16[natural=tree], way|z16[natural=tree_row] {"
        "tree-step: 3m;"
        "lsys: tree;"
        "lsys-size: 1m;"
        "lsys-colors: gray,yellow;"
        "lsys-texture-indices: 0,0;"
        "lsys-texture-types: background,grass;"
        "lsys-texture-scales: 50,200;"
        "}";

struct Builders_Poi_TreeBuilderFixture {
  Builders_Poi_TreeBuilderFixture() :
      dependencyProvider(),
      context(
          QuadKey(16, 35204, 21494),
          *dependencyProvider.getStyleProvider(createStyleSheet()),
          *dependencyProvider.getStringTable(),
          *dependencyProvider.getElevationProvider(),
          [&](const Mesh &mesh) {
            isCalled = true;
            BOOST_CHECK_GT(mesh.vertices.size(), 0);
            BOOST_CHECK_GT(mesh.triangles.size(), 0);
            BOOST_CHECK_GT(mesh.colors.size(), 0);
          },
          nullptr,
          dependencyProvider.getCancellationToken()),
      isCalled(false) {}

  StyleSheet createStyleSheet() const {
    auto stylesheet = utymap::mapcss::MapCssParser().parse(stylesheetStr);
    LSystem lsystem;
    lsystem.axiom.push_back(std::make_shared<MoveForwardRule>());
    stylesheet.lsystems.emplace("tree", lsystem);
    return stylesheet;
  }

  DependencyProvider dependencyProvider;
  BuilderContext context;
  bool isCalled;
};
}

BOOST_FIXTURE_TEST_SUITE(Builders_Poi_TreeBuilder, Builders_Poi_TreeBuilderFixture)

BOOST_AUTO_TEST_CASE(GivenTree_WhenVisitNode_ThenMeshIsBuilt) {
  Node tree = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0, {{"natural", "tree"}});
  tree.coordinate = GeoCoordinate(52.5137977, 13.3818357);
  TreeBuilder builder(context);

  builder.visitNode(tree);

  BOOST_CHECK(isCalled);
}

BOOST_AUTO_TEST_CASE(GivenTreeRow_WhenVisitWay_ThenMeshIsBuilt) {
  Way treeRow = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), 0,
                                                 {{"natural", "tree_row"}},
                                                 {{52.5137977, 13.3818357}, {52.5130465, 13.3822282},
                                                  {52.5129842, 13.3819065}});
  TreeBuilder builder(context);

  builder.visitWay(treeRow);

  BOOST_CHECK(isCalled);
}

BOOST_AUTO_TEST_SUITE_END()