#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "index/ElementStore.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::tests;

namespace {
typedef std::function<void(const Element &, const QuadKey &)> StoreCallback;

class TestElementStore : public ElementStore {
 public:
  int times;

  TestElementStore(StringTable &stringTable, StoreCallback function) :
      ElementStore(stringTable),
      times(0),
      function_(function) {
  }

  void search(const QuadKey &, ElementVisitor &, const CancellationToken &) override {}

  bool hasData(const QuadKey &) const override { return true; }

 protected:

  void storeImpl(const Element &element, const QuadKey &quadKey) override {
    times++;
    function_(element, quadKey);
  }
 private:
  StoreCallback function_;
};

struct Index_ElementStoreFixture {
  DependencyProvider dependencyProvider;
};

bool checkQuadKey(const utymap::QuadKey &quadKey, int lod, int tileX, int tileY) {
  return quadKey.levelOfDetail==lod && quadKey.tileX==tileX && quadKey.tileY==tileY;
}

template<typename T>
void checkGeometry(const T &t, std::initializer_list<std::pair<double, double>> geometry) {
  BOOST_CHECK_EQUAL(t.coordinates.size(), geometry.size());
  int i = 0;
  for (const auto &pair : geometry) {
    BOOST_CHECK_EQUAL(pair.first, t.coordinates[i].latitude);
    BOOST_CHECK_EQUAL(pair.second, t.coordinates[i].longitude);
    i++;
  }
}
}

BOOST_FIXTURE_TEST_SUITE(Index_ElementStore, Index_ElementStoreFixture)

BOOST_AUTO_TEST_CASE(GivenWayIntersectsTwoTilesOnce_WhenStore_GeometryIsClipped) {
  Way way = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), 0,
                                             {{"test", "Foo"}},
                                             {{10, 10}, {10, -10}});
  TestElementStore elementStore(*dependencyProvider.getStringTable(),
    [&](const Element &element, const QuadKey &quadKey) {
      if (checkQuadKey(quadKey, 1, 0, 0)) {
        checkGeometry<Way>(static_cast<const Way &>(element), {{10, -10}, {10, 0}});
      } else if (checkQuadKey(quadKey, 1, 1, 0)) {
        checkGeometry<Way>(static_cast<const Way &>(element), {{10, 0}, {10, 10}});
      } else {
        BOOST_FAIL("Unexpected quadKey!");
      }
                                });

  elementStore.store(way, LodRange(1, 1),
                     *dependencyProvider.getStyleProvider("way|z1[test=Foo] { key:val; clip: true;}"));

  BOOST_CHECK_EQUAL(elementStore.times, 2);
}

BOOST_AUTO_TEST_CASE(GivenWayIntersectsTwoTilesTwice_WhenStore_GeometryIsClipped) {
  Way way = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), 0,
                                             {{"test", "Foo"}},
                                             {{10, 10}, {10, -10}, {20, -10}, {20, 10}});
  TestElementStore elementStore(*dependencyProvider.getStringTable(),
      [&](const Element &element, const QuadKey &quadKey) {
        if (checkQuadKey(quadKey, 1, 0, 0)) {
          checkGeometry<Way>(static_cast<const Way &>(element),
                              {{20, 0}, {20, -10}, {10, -10}, {10, 0}});
        } else if (checkQuadKey(quadKey, 1, 1, 0)) {
          const Relation &relation = static_cast<const Relation &>(element);
          BOOST_CHECK_EQUAL(relation.elements.size(), 2);
          checkGeometry<Way>(static_cast<const Way &>(*relation.elements[0]),
                              {{20, 10}, {20, 0}});
          checkGeometry<Way>(static_cast<const Way &>(*relation.elements[1]),
                              {{10, 0}, {10, 10}});
        } else {
          BOOST_FAIL("Unexpected quadKey!");
        }
      });

  elementStore.store(way, LodRange(1, 1),
                     *dependencyProvider.getStyleProvider("way|z1[test=Foo] { key:val; clip: true;}"));

  BOOST_CHECK_EQUAL(elementStore.times, 2);
}

BOOST_AUTO_TEST_CASE(GivenWayOutsideTileWithBoundingBoxIntersectingTile_WhenStore_IsSkipped) {
  Way way = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), 0,
                                             {{"test", "Foo"}},
                                             {{-10, 20}, {-5, -10}, {10, -10}});
  TestElementStore elementStore(*dependencyProvider.getStringTable(),
      [&](const Element &element, const QuadKey &quadKey) {
        if (checkQuadKey(quadKey, 1, 1, 1) ||
            checkQuadKey(quadKey, 1, 0, 0) ||
            checkQuadKey(quadKey, 1, 0, 1)) {
          BOOST_CHECK(static_cast<const Way &>(element).coordinates.size() > 0);
        } else if (checkQuadKey(quadKey, 1, 1, 0)) {
          BOOST_FAIL("This quadkey should be skipped!!");
        } else {
          BOOST_FAIL("Unexpected quadKey!");
        }
      });

  elementStore.store(way, LodRange(1, 1),
                     *dependencyProvider.getStyleProvider("way|z1[test=Foo] { key:val; clip: true;}"));

  BOOST_CHECK_EQUAL(elementStore.times, 3);
}

BOOST_AUTO_TEST_CASE(GivenAreaIntersectsTwoTilesOnce_WhenStore_GeometryIsClipped) {
  Area area = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), 0,
                                                {{"test", "Foo"}},
                                                {{10, 10}, {20, 10}, {20, -10}, {10, -10}});
  TestElementStore elementStore(*dependencyProvider.getStringTable(),
    [&](const Element &element, const QuadKey &quadKey) {
      if (checkQuadKey(quadKey, 1, 0, 0)) {
        checkGeometry<Area>(static_cast<const Area &>(element),
                            {{20, 0}, {20, -10}, {10, -10}, {10, 0}});
      } else if (checkQuadKey(quadKey, 1, 1, 0)) {
        checkGeometry<Area>(static_cast<const Area &>(element),
                            {{20, 10}, {20, 0}, {10, 0}, {10, 10}});
      } else {
        BOOST_FAIL("Unexpected quadKey!");
      }
    });

  elementStore.store(area, LodRange(1, 1),
                     *dependencyProvider.getStyleProvider("area|z1[test=Foo] { key:val; clip: true;}"));

  BOOST_CHECK_EQUAL(elementStore.times, 2);
}

BOOST_AUTO_TEST_CASE(GivenAreaIntersectsTwoTilesTwice_WhenStore_GeometryIsClipped) {
  Area area = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), 0,
                                                {{"test", "Foo"}},
                                                {{20, 10}, {20, -10}, {5, -10}, {5, 10}, {10, 10}, {10, -5}, {15, -5},
                                                 {15, 10}});
  TestElementStore elementStore(*dependencyProvider.getStringTable(),
      [&](const Element &element, const QuadKey &quadKey) {
        if (checkQuadKey(quadKey, 1, 0, 0)) {
          checkGeometry<Area>(static_cast<const Area &>(element),
                              {{10, 0}, {10, -5}, {15, -5}, {15, 0}, {20, 0}, {20, -10},
                                {5, -10}, {5, 0}});
        } else if (checkQuadKey(quadKey, 1, 1, 0)) {
          const Relation &relation = static_cast<const Relation &>(element);
          BOOST_CHECK_EQUAL(relation.elements.size(), 2);
          checkGeometry<Area>(static_cast<const Area &>(*relation.elements[0]),
                              {{15, 10}, {20, 10}, {20, 0}, {15, 0}});
          checkGeometry<Area>(static_cast<const Area &>(*relation.elements[1]),
                              {{10, 10}, {10, 0}, {5, 0}, {5, 10}});
        } else {
          BOOST_FAIL("Unexpected quadKey!");
        }
      });

  elementStore.store(area, LodRange(1, 1),
                     *dependencyProvider.getStyleProvider("area|z1[test=Foo] { key:val; clip: true;}"));

  BOOST_CHECK_EQUAL(elementStore.times, 2);
}

BOOST_AUTO_TEST_CASE(GivenAreaBiggerThanTile_WhenStore_GeometryIsTheSameAsForTile) {
  Area area = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), 0,
                                                {{"test", "Foo"}},
                                                {{-10, -10}, {-10, 30}, {-10, 181}, {91, 181}, {91, 100}, {91, -10}});
  TestElementStore elementStore(*dependencyProvider.getStringTable(),
      [&](const Element &element, const QuadKey &quadKey) {
        if (checkQuadKey(quadKey, 1, 1, 0)) {
          // TODO improve geometry check
          BOOST_CHECK_EQUAL(static_cast<const Area &>(element).coordinates.size(), 4);
        }
      });

  elementStore.store(area, QuadKey(1, 1, 0),
                     *dependencyProvider.getStyleProvider("area|z1[test=Foo] { key:val; clip: true;}"));

  BOOST_CHECK_EQUAL(elementStore.times, 1);
}

BOOST_AUTO_TEST_CASE(GivenRelationOfPolygonWithHole_WhenStore_RelationIsReturnedWithClippedGeometry) {
  Area outer = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), 0, {},
                                                 {{5, 10}, {20, 10}, {20, -10}, {5, -10}});
  Area inner = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), 0, {},
                                                 {{10, 5}, {15, 5}, {15, -5}, {10, -5}});
  Relation relation;
  relation.id = 0;
  relation.tags = std::vector<Tag>{ElementUtils::createTag(*dependencyProvider.getStringTable(), "test", "Foo")};
  relation.elements.push_back(std::make_shared<Area>(inner));
  relation.elements.push_back(std::make_shared<Area>(outer));
  TestElementStore elementStore(*dependencyProvider.getStringTable(),
      [&](const Element &element, const QuadKey &quadKey) {
        auto result = static_cast<const Relation &>(element);
        if (checkQuadKey(quadKey, 1, 1, 0)) {
          checkGeometry<Area>(static_cast<const Area &>(*result.elements[0]),
                              {{15, 5}, {15, 0}, {10, 0}, {10, 5}});
          checkGeometry<Area>(static_cast<const Area &>(*result.elements[1]),
                              {{20, 10}, {20, 0}, {5, 0}, {5, 10}});
        } else if (checkQuadKey(quadKey, 1, 0, 0)) {
          checkGeometry<Area>(static_cast<const Area &>(*result.elements[0]),
                              {{15, 0}, {15, -5}, {10, -5}, {10, 0}});
          checkGeometry<Area>(static_cast<const Area &>(*result.elements[1]),
                              {{20, 0}, {20, -10}, {5, -10}, {5, 0}});
        } else {
          BOOST_FAIL("Unexpected quadKey!");
        }
      });

  elementStore.store(relation, LodRange(1, 1),
                     *dependencyProvider.getStyleProvider("relation|z1[test=Foo] { key:val; clip: true;}"));

  BOOST_CHECK_EQUAL(elementStore.times, 2);
}

BOOST_AUTO_TEST_CASE(GivenRelationOfPolygonWithHole_WhenStore_RelationAndAreaReturnedWithClippedGeometry) {
  Area outer = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), 0, {},
                                                 {{5, 10}, {20, 10}, {20, -10}, {5, -10}});
  Area inner = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), 0, {},
                                                 {{10, 8}, {15, 8}, {15, 2}, {10, 2}});
  Relation relation;
  relation.tags = std::vector<Tag>{ElementUtils::createTag(*dependencyProvider.getStringTable(), "test", "Foo")};
  relation.elements.push_back(std::make_shared<Area>(inner));
  relation.elements.push_back(std::make_shared<Area>(outer));
  TestElementStore elementStore(*dependencyProvider.getStringTable(),
      [&](const Element &element, const QuadKey &quadKey) {
        if (checkQuadKey(quadKey, 1, 1, 0)) {
          const Relation &result = static_cast<const Relation &>(element);
          BOOST_CHECK_EQUAL(result.elements.size(), 2);
          checkGeometry<Area>(static_cast<const Area &>(*result.elements[0]),
                              {{10, 8}, {15, 8}, {15, 2}, {10, 2}});
          checkGeometry<Area>(static_cast<const Area &>(*result.elements[1]),
                              {{20, 10}, {20, 0}, {5, 0}, {5, 10}});
        } else if (checkQuadKey(quadKey, 1, 0, 0)) {
          checkGeometry<Area>(static_cast<const Area &>(element),
                              {{20, 0}, {20, -10}, {5, -10}, {5, 0}});
        } else {
          BOOST_FAIL("Unexpected quadKey!");
        }
      });

  elementStore.store(relation, LodRange(1, 1),
                     *dependencyProvider.getStyleProvider("relation|z1[test=Foo] { key:val; clip: true;}"));

  BOOST_CHECK_EQUAL(elementStore.times, 2);
}

BOOST_AUTO_TEST_CASE(GivenWay_WhenStoreInsideQuadKey_IsStored) {
  Way way = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), 0,
                                             {{"test", "Foo"}}, {{5, 5}, {10, 10}});
  TestElementStore elementStore(*dependencyProvider.getStringTable(),
      [&](const Element &element, const QuadKey &quadKey) {
        if (checkQuadKey(quadKey, 1, 1, 0)) {
          BOOST_CHECK(static_cast<const Way &>(element).coordinates.size() > 0);
        } else {
          BOOST_FAIL("Unexpected quadKey!");
        }
      });

  elementStore.store(way, LodRange(1, 1),
                     *dependencyProvider.getStyleProvider("way|z1[test=Foo] { key:val; clip: true;}"));

  BOOST_CHECK_EQUAL(elementStore.times, 1);
}

BOOST_AUTO_TEST_SUITE_END()
