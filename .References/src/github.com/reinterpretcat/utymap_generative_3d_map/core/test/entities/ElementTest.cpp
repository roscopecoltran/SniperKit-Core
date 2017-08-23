#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap::entities;
using namespace utymap::tests;

namespace {
struct Counter : public ElementVisitor {
  int nodes, ways, areas, relations;

  Counter() : nodes(0), ways(0), areas(0), relations(0) {}

  void visitNode(const Node &) override { nodes++; }
  void visitWay(const Way &) override { ways++; }
  void visitArea(const Area &) override { areas++; }
  void visitRelation(const Relation &) override { relations++; }
};

struct Entities_ElementFixture {
  DependencyProvider dependencyProvider;
};
}

BOOST_FIXTURE_TEST_SUITE(Entities_Element, Entities_ElementFixture)

BOOST_AUTO_TEST_CASE(GivenNode_WhenVisit_ThenIncrementsCounter) {
  Counter counter;
  Node node;

  node.accept(counter);

  BOOST_CHECK_EQUAL(counter.nodes, 1);
  BOOST_CHECK_EQUAL(counter.ways, 0);
  BOOST_CHECK_EQUAL(counter.areas, 0);
  BOOST_CHECK_EQUAL(counter.relations, 0);
}

BOOST_AUTO_TEST_CASE(GivenNodeWithTwoTags_WhenToString_ThenReturnsValidRepresentation) {
  Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 1,
                                                {{"key1", "value1"}, {"key2", "value2"}});

  std::string result = node.toString(*dependencyProvider.getStringTable());

  BOOST_CHECK_EQUAL(result, "[1]{key1:value1,key2:value2,}");
}

BOOST_AUTO_TEST_CASE(GivenWay_WhenVisit_ThenIncrementsCounter) {
  Counter counter;
  Way way;

  way.accept(counter);

  BOOST_CHECK_EQUAL(counter.nodes, 0);
  BOOST_CHECK_EQUAL(counter.ways, 1);
  BOOST_CHECK_EQUAL(counter.areas, 0);
  BOOST_CHECK_EQUAL(counter.relations, 0);
}

BOOST_AUTO_TEST_CASE(GivenArea_WhenVisit_ThenIncrementsCounter) {
  Counter counter;
  Area area;

  area.accept(counter);

  BOOST_CHECK_EQUAL(counter.nodes, 0);
  BOOST_CHECK_EQUAL(counter.ways, 0);
  BOOST_CHECK_EQUAL(counter.areas, 1);
  BOOST_CHECK_EQUAL(counter.relations, 0);
}

BOOST_AUTO_TEST_CASE(GivenRelation_WhenVisit_ThenIncrementsCounter) {
  Counter counter;
  Relation relation;

  relation.accept(counter);

  BOOST_CHECK_EQUAL(counter.nodes, 0);
  BOOST_CHECK_EQUAL(counter.ways, 0);
  BOOST_CHECK_EQUAL(counter.areas, 0);
  BOOST_CHECK_EQUAL(counter.relations, 1);
}

BOOST_AUTO_TEST_SUITE_END()
