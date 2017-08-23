#include "builders/misc/BarrierBuilder.hpp"
#include "entities/Way.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::math;
using namespace utymap::tests;

namespace {
const std::string stylesheet = "way|z16[barrier] {"
    "height:2m; min-height:0m;"
    "color:gradient(red);"
    "offset:0.2m;"
    "}";
struct Builders_Misc_BarrierBuilderFixture {
  DependencyProvider dependencyProvider;
};
}

BOOST_FIXTURE_TEST_SUITE(Builders_Misc_BarrierBuilder, Builders_Misc_BarrierBuilderFixture)

BOOST_AUTO_TEST_CASE(GivenBarrier_WhenVisitWay_ThenMeshIsBuilt) {
  bool isCalled = false;
  auto context = dependencyProvider.createBuilderContext(QuadKey(16, 1, 1), stylesheet,
                                                         [&](const Mesh &mesh) {
                                                           isCalled = true;
                                                           BOOST_CHECK_GT(mesh.vertices.size(), 0);
                                                           BOOST_CHECK_GT(mesh.triangles.size(), 0);
                                                           BOOST_CHECK_GT(mesh.colors.size(), 0);
                                                         });
  BarrierBuilder builder(*context);
  Way way = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), 0,
                                             {{"barrier", "yes"}},
                                             {{0, 0}, {0, 10}, {10, 10}, {10, 0}});

  builder.visitWay(way);

  BOOST_CHECK(isCalled);
}

BOOST_AUTO_TEST_SUITE_END()
