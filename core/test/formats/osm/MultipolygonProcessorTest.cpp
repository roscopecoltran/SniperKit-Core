#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/osm/MultipolygonProcessor.hpp"
#include "test_utils/ElementUtils.hpp"
#include "utils/GeometryUtils.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::index;
using namespace utymap::tests;

namespace {

struct Formats_Osm_MultipolygonProcessorFixture {
  Formats_Osm_MultipolygonProcessorFixture()
      : dependencyProvider(), context() {
  }

  std::shared_ptr<Relation> createRelation() {
    auto relation = std::make_shared<Relation>();
    relation->id = 0;
    context.relationMap[0] = relation;
    return relation;
  }

  RelationMembers createRelationMembers(const std::initializer_list<std::tuple<std::uint64_t,
                                                                               std::string,
                                                                               std::string>> &membersInfo) const {
    RelationMembers members;
    members.reserve(membersInfo.size());
    for (const auto &info : membersInfo) {
      members.push_back(RelationMember{std::get<0>(info), std::get<1>(info), std::get<2>(info)});
    }
    return std::move(members);
  }

  void resolve(Relation &relation) {
  }

  std::vector<GeoCoordinate> ensureExpectedOrientation(std::vector<GeoCoordinate> &source, bool isOuter = true) const {
    std::vector<GeoCoordinate> destination;
    destination.reserve(source.size());
    bool isClockwise = utymap::utils::isClockwise(source);
    if ((isClockwise && !isOuter) || (!isClockwise && isOuter))
      destination.insert(destination.end(), source.begin(), source.end());
    else
      destination.insert(destination.end(), source.rbegin(), source.rend());

    return std::move(destination);
  }

  template<typename T>
  std::shared_ptr<T> createElement(std::initializer_list<std::pair<double, double>> geometry) {
    return std::make_shared<T>(ElementUtils::createElement<T>(*dependencyProvider.getStringTable(), 0, {}, geometry));
  }

  DependencyProvider dependencyProvider;
  OsmDataContext context;
};
}

BOOST_FIXTURE_TEST_SUITE(Formats_Osm_MultipolygonProcessor, Formats_Osm_MultipolygonProcessorFixture)

BOOST_AUTO_TEST_CASE(GivenOneOuterOneInnerAllClosed_WhenProcess_ThenReturnCorrectResult) {
  RelationMembers relationMembers = createRelationMembers({
                                                              std::make_tuple(1, "w", "outer"),
                                                              std::make_tuple(2, "w", "inner")
                                                          });
  context.areaMap[1] = createElement<Area>({{0, 0}, {3, 5}, {7, 3}, {8, -1}, {3, -4}});
  context.areaMap[2] = createElement<Area>({{2, 0}, {3, 2}, {5, 1}, {4, -1}});
  MultipolygonProcessor processor(*createRelation(), relationMembers, context,
                                  std::bind(&Formats_Osm_MultipolygonProcessorFixture::resolve,
                                            this,
                                            std::placeholders::_1));

  processor.process();

  auto relation = context.relationMap[0];
  BOOST_CHECK_EQUAL(2, relation->elements.size());

  BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area &>(*relation->elements[0]).coordinates.size());
  BOOST_CHECK(ensureExpectedOrientation(context.areaMap[1]->coordinates)
                  ==reinterpret_cast<const Area &>(*relation->elements[0]).coordinates);

  BOOST_CHECK_EQUAL(4, reinterpret_cast<const Area &>(*relation->elements[1]).coordinates.size());
  BOOST_CHECK(ensureExpectedOrientation(context.areaMap[2]->coordinates, false)
                  ==reinterpret_cast<const Area &>(*relation->elements[1]).coordinates);
}

BOOST_AUTO_TEST_CASE(GivenOneOuterTwoInnerAllClosed_WhenProcess_ThenReturnCorrectResult) {
  RelationMembers relationMembers = createRelationMembers({
                                                              std::make_tuple(1, "w", "outer"),
                                                              std::make_tuple(2, "w", "inner"),
                                                              std::make_tuple(3, "w", "inner"),
                                                          });
  context.areaMap[1] = createElement<Area>({{0, 0}, {3, 5}, {7, 3}, {8, -1}, {3, -4}});
  context.areaMap[2] = createElement<Area>({{2, 1}, {3, 3}, {5, 2}, {4, 0}});
  context.areaMap[3] = createElement<Area>({{3, -1}, {5, -1}, {3, -3}, {2, -2}});
  MultipolygonProcessor processor(*createRelation(), relationMembers, context,
                                  std::bind(&Formats_Osm_MultipolygonProcessorFixture::resolve,
                                            this,
                                            std::placeholders::_1));

  processor.process();

  auto relation = context.relationMap[0];
  BOOST_CHECK_EQUAL(3, relation->elements.size());

  BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area &>(*relation->elements[0]).coordinates.size());
  BOOST_CHECK(ensureExpectedOrientation(context.areaMap[1]->coordinates)
                  ==reinterpret_cast<const Area &>(*relation->elements[0]).coordinates);

  BOOST_CHECK_EQUAL(4, reinterpret_cast<const Area &>(*relation->elements[1]).coordinates.size());
  BOOST_CHECK(ensureExpectedOrientation(context.areaMap[2]->coordinates, false)
                  ==reinterpret_cast<const Area &>(*relation->elements[1]).coordinates);

  BOOST_CHECK_EQUAL(4, reinterpret_cast<const Area &>(*relation->elements[2]).coordinates.size());
  BOOST_CHECK(ensureExpectedOrientation(context.areaMap[3]->coordinates, false)
                  ==reinterpret_cast<const Area &>(*relation->elements[2]).coordinates);
}

BOOST_AUTO_TEST_CASE(GivenOneOuterNonClosed_WhenProcess_ThenReturnCorrectResult) {
  RelationMembers relationMembers = createRelationMembers({
                                                              std::make_tuple(1, "w", "outer"),
                                                              std::make_tuple(2, "w", "outer")
                                                          });
  context.wayMap[1] = createElement<Way>({{0, 0}, {3, 5}, {7, 3}});
  context.wayMap[2] = createElement<Way>({{7, 3}, {8, -1}, {3, -4}, {0, 0}});
  MultipolygonProcessor processor(*createRelation(), relationMembers, context,
                                  std::bind(&Formats_Osm_MultipolygonProcessorFixture::resolve,
                                            this,
                                            std::placeholders::_1));

  processor.process();

  auto relation = context.relationMap[0];
  BOOST_CHECK_EQUAL(1, relation->elements.size());
  BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area &>(*relation->elements[0]).coordinates.size());
  std::vector<GeoCoordinate> expected = {{7, 3}, {8, -1}, {3, -4}, {0, 0}, {3, 5}};
  BOOST_CHECK(ensureExpectedOrientation(expected)==reinterpret_cast<const Area &>(*relation->elements[0]).coordinates);
}

BOOST_AUTO_TEST_CASE(GivenTwoOuterClosed_WhenProcess_ThenReturnCorrectResult) {
  RelationMembers relationMembers = createRelationMembers({
                                                              std::make_tuple(1, "w", "outer"),
                                                              std::make_tuple(2, "w", "outer")
                                                          });
  context.areaMap[1] = createElement<Area>({{0, 0}, {3, 5}, {7, 3}, {8, -1}, {3, -4}});
  context.areaMap[2] = createElement<Area>({{10, -3}, {14, -3}, {14, -6}, {10, -6}});
  MultipolygonProcessor processor(*createRelation(), relationMembers, context,
                                  std::bind(&Formats_Osm_MultipolygonProcessorFixture::resolve,
                                            this,
                                            std::placeholders::_1));

  processor.process();

  auto relation = context.relationMap[0];
  BOOST_CHECK_EQUAL(2, relation->elements.size());
  BOOST_CHECK(ensureExpectedOrientation(context.areaMap[1]->coordinates)
                  ==reinterpret_cast<const Area &>(*relation->elements[1]).coordinates);
  BOOST_CHECK(ensureExpectedOrientation(context.areaMap[2]->coordinates)
                  ==reinterpret_cast<const Area &>(*relation->elements[0]).coordinates);
}

BOOST_AUTO_TEST_CASE(GivenOneOuterNonClosedAndTwoInnerClosed_WhenProcess_ThenReturnCorrectResult) {
  RelationMembers relationMembers = createRelationMembers({
                                                              std::make_tuple(1, "w", "outer"),
                                                              std::make_tuple(2, "w", "outer"),
                                                              std::make_tuple(3, "w", "inner"),
                                                              std::make_tuple(4, "w", "inner")
                                                          });
  context.wayMap[1] = createElement<Way>({{0, 0}, {3, 5}, {7, 3}});
  context.wayMap[2] = createElement<Way>({{7, 3}, {8, -1}, {3, -4}, {0, 0}});
  context.areaMap[3] = createElement<Area>({{2, 1}, {3, 3}, {5, 2}, {4, 0}});
  context.areaMap[4] = createElement<Area>({{3, -1}, {5, -1}, {3, -3}, {2, -2}});
  MultipolygonProcessor processor(*createRelation(), relationMembers, context,
                                  std::bind(&Formats_Osm_MultipolygonProcessorFixture::resolve,
                                            this,
                                            std::placeholders::_1));

  processor.process();

  auto relation = context.relationMap[0];
  BOOST_CHECK_EQUAL(3, relation->elements.size());

  BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area &>(*relation->elements[0]).coordinates.size());
  std::vector<GeoCoordinate> expected = {{7, 3}, {8, -1}, {3, -4}, {0, 0}, {3, 5}};
  BOOST_CHECK(ensureExpectedOrientation(expected)==reinterpret_cast<const Area &>(*relation->elements[0]).coordinates);

  BOOST_CHECK_EQUAL(4, reinterpret_cast<const Area &>(*relation->elements[2]).coordinates.size());
  BOOST_CHECK(ensureExpectedOrientation(context.areaMap[3]->coordinates, false)
                  ==reinterpret_cast<const Area &>(*relation->elements[2]).coordinates);

  BOOST_CHECK_EQUAL(4, reinterpret_cast<const Area &>(*relation->elements[1]).coordinates.size());
  BOOST_CHECK(ensureExpectedOrientation(context.areaMap[4]->coordinates, false)
                  ==reinterpret_cast<const Area &>(*relation->elements[1]).coordinates);
}

BOOST_AUTO_TEST_CASE(GivenMultiplyOuterAndMultiplyInner_WhenProcess_ThenReturnCorrectResult) {
  // see fig.6 http://wiki.openstreetmap.org/wiki/Talk:Relation:multipolygon

  RelationMembers relationMembers = createRelationMembers({
                                                              std::make_tuple(1, "w", "outer"),
                                                              std::make_tuple(2, "w", "outer"),
                                                              std::make_tuple(3, "w", "outer"),
                                                              std::make_tuple(4, "w", "outer"),

                                                              std::make_tuple(5, "w", "inner"),
                                                              std::make_tuple(6, "w", "inner"),
                                                              std::make_tuple(7, "w", "inner"),
                                                              std::make_tuple(8, "w", "inner"),
                                                              std::make_tuple(9, "w", "inner"),
                                                              std::make_tuple(10, "w", "inner"),
                                                              std::make_tuple(11, "w", "inner"),

                                                              std::make_tuple(12, "w", "outer"),
                                                              std::make_tuple(13, "w", "outer"),
                                                              std::make_tuple(14, "w", "outer"),
                                                              std::make_tuple(15, "w", "outer"),

                                                              std::make_tuple(16, "w", "inner"),
                                                              std::make_tuple(17, "w", "inner"),
                                                              std::make_tuple(18, "w", "inner"),
                                                              std::make_tuple(19, "w", "inner"),

                                                              std::make_tuple(20, "w", "outer")
                                                          });
  context.wayMap[1] = createElement<Way>({{1, 5}, {8, 4}});
  context.wayMap[2] = createElement<Way>({{8, 4}, {9, -1}});
  context.wayMap[3] = createElement<Way>({{9, -1}, {8, -6}, {2, -5}});
  context.wayMap[4] = createElement<Way>({{2, -5}, {0, -3}, {1, 5}});

  context.wayMap[5] = createElement<Way>({{2, 1}, {3, 3}, {6, 3}});
  context.wayMap[6] = createElement<Way>({{6, 3}, {4, 0}, {2, 1}});
  context.wayMap[7] = createElement<Way>({{1, -2}, {3, -1}});
  context.wayMap[8] = createElement<Way>({{3, -1}, {4, -4}});
  context.wayMap[9] = createElement<Way>({{4, -4}, {1, -3}});
  context.wayMap[10] = createElement<Way>({{1, -3}, {1, -2}});
  context.areaMap[11] = createElement<Area>({{6, -3}, {7, -1}, {8, -4}});

  context.wayMap[12] = createElement<Way>({{10, 5}, {14, 5}});
  context.wayMap[13] = createElement<Way>({{14, 5}, {14, -1}});
  context.wayMap[14] = createElement<Way>({{14, -1}, {10, -1}});
  context.wayMap[15] = createElement<Way>({{10, -1}, {10, 5}});

  context.wayMap[16] = createElement<Way>({{11, 4}, {13, 4}});
  context.wayMap[17] = createElement<Way>({{13, 4}, {13, 0}, {11, 0}});
  context.wayMap[18] = createElement<Way>({{11, 0}, {12, 2}});
  context.wayMap[19] = createElement<Way>({{12, 2}, {11, 4}});

  context.areaMap[20] = createElement<Area>({{10, -3}, {14, -3}, {14, -6}, {10, -6}});
  MultipolygonProcessor processor(*createRelation(), relationMembers, context,
                                  std::bind(&Formats_Osm_MultipolygonProcessorFixture::resolve,
                                            this,
                                            std::placeholders::_1));

  processor.process();

  auto relation = context.relationMap[0];
  BOOST_CHECK_EQUAL(7, relation->elements.size());
}

// Reproducing crash.
BOOST_AUTO_TEST_CASE(GivenSpecificFourOuter_WhenProcess_ThenDoesNotCrash) {
  RelationMembers relationMembers = createRelationMembers({
                                                              std::make_tuple(1, "w", "outer"),
                                                              std::make_tuple(2, "w", "outer"),
                                                              std::make_tuple(3, "w", "outer"),
                                                              std::make_tuple(4, "w", "outer")
                                                          });
  context.wayMap[1] =
      createElement<Way>({{55.754873400000001, 37.620234000000004}, {55.754922700000002, 37.620224499999999}});
  context.wayMap[2] =
      createElement<Way>({{55.754873400000001, 37.620234000000004}, {55.754846999999998, 37.620192099999997}});
  context.wayMap[3] =
      createElement<Way>({{55.754846999999998, 37.620192099999997}, {55.754846100000002, 37.620103100000001}});
  context.wayMap[4] =
      createElement<Way>({{55.754846100000002, 37.620103100000001}, {55.754922700000002, 37.620224499999999}});

  MultipolygonProcessor processor(*createRelation(), relationMembers, context,
                                  std::bind(&Formats_Osm_MultipolygonProcessorFixture::resolve,
                                            this,
                                            std::placeholders::_1));

  processor.process();

  auto relation = context.relationMap[0];
  BOOST_CHECK_EQUAL(1, relation->elements.size());
  BOOST_CHECK_EQUAL(4, reinterpret_cast<const Area &>(*relation->elements[0]).coordinates.size());
}

BOOST_AUTO_TEST_SUITE_END()
