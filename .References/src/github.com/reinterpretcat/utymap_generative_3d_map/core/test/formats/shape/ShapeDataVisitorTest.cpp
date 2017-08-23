#include "config.hpp"
#include "formats/shape/ShapeParser.hpp"
#include "formats/shape/ShapeDataVisitor.hpp"
#include "index/InMemoryElementStore.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::formats;
using namespace utymap::mapcss;
using namespace utymap::tests;

namespace {
const std::string stylesheet = "way|z1-16[test=Foo] { key:val; }";

struct Formats_Shape_ShapeDataVisitorFixture {
  Formats_Shape_ShapeDataVisitorFixture() :
      shapeFile(TEST_SHAPE_LINE_FILE),
      dependencyProvider(),
      elementStore(*dependencyProvider.getStringTable()) {
  }

  std::string shapeFile;
  DependencyProvider dependencyProvider;
  InMemoryElementStore elementStore;
};
}

BOOST_FIXTURE_TEST_SUITE(Formats_Shape_ShapeDataVisitor, Formats_Shape_ShapeDataVisitorFixture)

BOOST_AUTO_TEST_CASE(GivenDefaultXml_WhenParserParse_ThenHasExpectedElementCount) {
  ShapeDataVisitor visitor(*dependencyProvider.getStringTable(), [&](Element &element) {
    return elementStore.store(element, utymap::LodRange(1, 1), *dependencyProvider.getStyleProvider(stylesheet));
  });
  ShapeParser<ShapeDataVisitor> parser;

  parser.parse(shapeFile, visitor);

  BOOST_CHECK_EQUAL(0, visitor.nodes);
  BOOST_CHECK_EQUAL(1, visitor.ways);
  BOOST_CHECK_EQUAL(0, visitor.areas);
  BOOST_CHECK_EQUAL(0, visitor.relations);
}

BOOST_AUTO_TEST_SUITE_END()
