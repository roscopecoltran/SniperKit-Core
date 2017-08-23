#include "BoundingBox.hpp"
#include "formats/osm/json/OsmJsonParser.hpp"
#include "formats/osm/CountableOsmDataVisitor.hpp"
#include "config.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"

using namespace utymap::formats;
using namespace utymap::tests;

namespace {
struct Formats_Osm_Json_OsmJsonParserFixture {
  Formats_Osm_Json_OsmJsonParserFixture() :
      parser(*provider.getStringTable()),
      istream(TEST_JSON_FILE, std::ios::in) {
  }
  DependencyProvider provider;
  OsmJsonParser<CountableOsmDataVisitor> parser;
  CountableOsmDataVisitor visitor;
  std::ifstream istream;
};
}

BOOST_FIXTURE_TEST_SUITE(Formats_Osm_Json_Parser, Formats_Osm_Json_OsmJsonParserFixture)

BOOST_AUTO_TEST_CASE(GivenDefaultJson_WhenParserParse_ThenHasExpectedElementCount) {
  parser.parse(istream, visitor);

  BOOST_CHECK_EQUAL(161, visitor.nodes);
  BOOST_CHECK_EQUAL(120, visitor.ways);
  BOOST_CHECK_EQUAL(453, visitor.areas);
  BOOST_CHECK_EQUAL(16, visitor.relations);
}

BOOST_AUTO_TEST_SUITE_END()
