#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "index/InMemoryElementStore.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::tests;

namespace {
const std::string stylesheet = "area|z1[any],way|z1[any],node|z1[any] { clip: true; }";

struct Index_InMemoryElementStoreFixture {
  Index_InMemoryElementStoreFixture() :
      dependencyProvider(),
      elementStore(*dependencyProvider.getStringTable()) {
    LodRange range(1, 2);

    auto styleProvider = dependencyProvider.getStyleProvider(stylesheet);

    elementStore.store(ElementUtils::createElement<Way>(
        *dependencyProvider.getStringTable(), 0,
        {{"any", "true"}}, {{5, -5}, {5, -10}}), range, *styleProvider);
    elementStore.store(ElementUtils::createElement<Area>(
        *dependencyProvider.getStringTable(), 0,
        {{"any", "true"}}, {{5, -5}, {5, -10}, {10, -10}}), range,*styleProvider);

    Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0,
                                                  {{"any", "true"}});
    node.coordinate = {5, -5};
    elementStore.store(node, range, *styleProvider);
  }

  DependencyProvider dependencyProvider;
  InMemoryElementStore elementStore;
};

struct ElementCounter : public ElementVisitor {
  int times = 0;

  void visitNode(const Node &) override { ++times; }
  void visitWay(const Way &) override { ++times; }
  void visitArea(const Area &) override { ++times; }
  void visitRelation(const Relation &) override { ++times; }
};
}

BOOST_FIXTURE_TEST_SUITE(Index_InMemoryElementStore, Index_InMemoryElementStoreFixture)

BOOST_AUTO_TEST_CASE(GivenNodeWayArea_WhenSearch_ThenAllFound) {
  QuadKey quadKey(1, 0, 0);
  ElementCounter counter;

  elementStore.search(quadKey, counter, CancellationToken());

  BOOST_CHECK_EQUAL(counter.times, 3);
}

BOOST_AUTO_TEST_CASE(GivenNodeWayArea_WhenSearch_ThenAllSkipped) {
  QuadKey quadKey(2, 0, 0);
  ElementCounter counter;

  elementStore.search(quadKey, counter, CancellationToken());

  BOOST_CHECK_EQUAL(counter.times, 0);
}

BOOST_AUTO_TEST_SUITE_END()
