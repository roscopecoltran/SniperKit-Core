#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "index/PersistentElementStore.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

#include <boost/filesystem/operations.hpp>

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::tests;

namespace {
const std::string TestZoomDirectory = "data/1";
const std::string stylesheet = "node|z1[any], way|z1[any], area|z1[any], relation|z1[any] { clip: false; }";

struct Index_PersistentElementStoreFixture {
  Index_PersistentElementStoreFixture() :
      dependencyProvider(),
      elementStore("", *dependencyProvider.getStringTable()) {
    boost::filesystem::create_directories(TestZoomDirectory);
  }

  ~Index_PersistentElementStoreFixture() {
    boost::filesystem::path dir(TestZoomDirectory);
    for (boost::filesystem::directory_iterator dirEnd, it(dir); it!=dirEnd; ++it) {
      boost::filesystem::remove_all(it->path());
    }
    boost::filesystem::remove(TestZoomDirectory);
  }

  DependencyProvider dependencyProvider;
  PersistentElementStore elementStore;
};

struct ElementCounter : public ElementVisitor {
  int times = 0;
  std::shared_ptr<Element> element;

  void visitNode(const Node &node) override {
    ++times;
    element = std::make_shared<Node>(node);
  }

  void visitWay(const Way &way) override {
    ++times;
    element = std::make_shared<Way>(way);
  }
  void visitArea(const Area &area) override {
    ++times;
    element = std::make_shared<Area>(area);
  }
  void visitRelation(const Relation &relation) override {
    ++times;
    element = std::make_shared<Relation>(relation);
  }
};

void assertGeometry(const GeoCoordinate &expected, const GeoCoordinate &actual) {
  BOOST_CHECK_EQUAL(expected.latitude, actual.latitude);
  BOOST_CHECK_EQUAL(expected.longitude, actual.longitude);
}

void assertGeometry(const std::vector<GeoCoordinate> &expected, const std::vector<GeoCoordinate> &actual) {
  BOOST_CHECK_EQUAL(expected.size(), actual.size());

  for (std::size_t i = 0; i < expected.size(); ++i) {
    assertGeometry(expected[i], actual[i]);
  }
}

void assertElement(const Element &expected, const Element &actual) {
  BOOST_CHECK_EQUAL(expected.id, actual.id);
  BOOST_CHECK_EQUAL(expected.tags.size(), actual.tags.size());
}

void assertNode(const Node &expected, const Node &actual) {
  assertElement(expected, actual);
  assertGeometry(expected.coordinate, actual.coordinate);
}

template<typename T>
void assertWayOrArea(const T &expected, const T &actual) {
  assertElement(expected, actual);
  assertGeometry(expected.coordinates, actual.coordinates);
}
}

BOOST_FIXTURE_TEST_SUITE(Index_PersistentElementStore, Index_PersistentElementStoreFixture)

BOOST_AUTO_TEST_CASE(GivenNode_WhenStoreAndSearch_ThenItIsStoredAndReadBack) {
  LodRange range(1, 2);
  QuadKey quadKey(1, 0, 0);
  auto styleProvider = dependencyProvider.getStyleProvider(stylesheet);
  Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 7, {{"any", "true"}});
  node.coordinate = {5, -5};
  ElementCounter counter;

  elementStore.store(node, range, *styleProvider);
  elementStore.search(quadKey, counter, CancellationToken());

  assertNode(node, *std::dynamic_pointer_cast<Node>(counter.element));
}

BOOST_AUTO_TEST_CASE(GivenWay_WhenStoreAndSearch_ThenItIsStoredAndReadBack) {
  LodRange range(1, 2);
  QuadKey quadKey(1, 0, 0);
  auto styleProvider = dependencyProvider.getStyleProvider(stylesheet);
  Way way =
      ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), 7, {{"any", "true"}}, {{1, -1}, {5, -5}});
  ElementCounter counter;

  elementStore.store(way, range, *styleProvider);
  elementStore.search(quadKey, counter, CancellationToken());

  assertWayOrArea(way, *std::dynamic_pointer_cast<Way>(counter.element));
}

BOOST_AUTO_TEST_CASE(GivenArea_WhenStoreAndSearch_ThenItIsStoredAndReadBack) {
  LodRange range(1, 2);
  QuadKey quadKey(1, 0, 0);
  auto styleProvider = dependencyProvider.getStyleProvider(stylesheet);
  Area area = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(),
                                                7,
                                                {{"any", "true"}},
                                                {{1, -1}, {5, -5}, {10, -10}});
  ElementCounter counter;

  elementStore.store(area, range, *styleProvider);
  elementStore.search(quadKey, counter, CancellationToken());

  assertWayOrArea(area, *std::dynamic_pointer_cast<Area>(counter.element));
}

BOOST_AUTO_TEST_CASE(GivenRelationWithDifferentElements_WhenStoreAndSearch_ThenItIsStoredAndReadBack) {
  LodRange range(1, 2);
  QuadKey quadKey(1, 0, 0);
  auto styleProvider = dependencyProvider.getStyleProvider(stylesheet);
  Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 1, {{"n", "1"}});
  node.coordinate = {0.5, -0.5};
  Way way = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), 2, {{"w", "2"}}, {{1, -1}, {2, -2}});
  Area area = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(),
                                                3,
                                                {{"a", "3"}},
                                                {{3, -3}, {4, -4}, {5, -5}});
  Relation relation = ElementUtils::createElement<Relation>(*dependencyProvider.getStringTable(), 4, {{"any", "true"}});
  relation.elements.push_back(std::make_shared<Node>(node));
  relation.elements.push_back(std::make_shared<Way>(way));
  relation.elements.push_back(std::make_shared<Area>(area));
  ElementCounter counter;

  elementStore.store(relation, range, *styleProvider);
  elementStore.search(quadKey, counter, CancellationToken());

  BOOST_CHECK_EQUAL(counter.times, 1);
  Relation result = *std::dynamic_pointer_cast<Relation>(counter.element);
  assertElement(relation, result);
}

BOOST_AUTO_TEST_CASE(GivenTwoAreas_WhenStoreAndSearchOnce_ThenTheyStoredTwiceAndSecondReturnedLast) {
  LodRange range(1, 2);
  QuadKey quadKey(1, 0, 0);
  auto styleProvider = dependencyProvider.getStyleProvider(stylesheet);
  Area area1 = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(),
                                                 1,
                                                 {{"any", "true"}},
                                                 {{4, -4}, {5, -5}, {6, -6}});
  Area area2 = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(),
                                                 2,
                                                 {{"any", "true"}},
                                                 {{1, -1}, {2, -2}, {3, -3}});
  ElementCounter counter;

  elementStore.store(area1, range, *styleProvider);
  elementStore.store(area2, range, *styleProvider);
  elementStore.search(quadKey, counter, CancellationToken());

  BOOST_CHECK_EQUAL(counter.times, 2);
  assertWayOrArea(area2, *std::dynamic_pointer_cast<Area>(counter.element));
}

BOOST_AUTO_TEST_SUITE_END()
