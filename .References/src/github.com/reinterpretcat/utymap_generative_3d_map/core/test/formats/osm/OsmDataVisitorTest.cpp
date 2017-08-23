#include "entities/Relation.hpp"
#include "formats/osm/OsmDataVisitor.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"

using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::tests;

namespace {
struct Formats_Osm_OsmDataVisitorFixture {
  DependencyProvider dependencyProvider;
  OsmDataVisitor visitor;

  Formats_Osm_OsmDataVisitorFixture() :
      dependencyProvider(),
      visitor(*dependencyProvider.getStringTable(),
              std::bind(&Formats_Osm_OsmDataVisitorFixture::add, this, std::placeholders::_1)) {
  }

  bool add(utymap::entities::Element &) { return false; }
};
}

BOOST_FIXTURE_TEST_SUITE(Formats_Osm_OsmDataVisitor, Formats_Osm_OsmDataVisitorFixture)

BOOST_AUTO_TEST_CASE(GivenRelationWithRecursion_WhenComplete_ThenDoesNotCrash) {
  Tags tags = {};
  RelationMembers parentMembers = {{2, "", ""}};
  RelationMembers childMembers = {{1, "", ""}};

  visitor.visitRelation(1, parentMembers, tags);
  visitor.visitRelation(2, childMembers, tags);

  visitor.complete();
}

BOOST_AUTO_TEST_SUITE_END()
