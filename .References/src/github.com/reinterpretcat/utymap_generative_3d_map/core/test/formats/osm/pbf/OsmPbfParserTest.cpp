#include "formats/osm/pbf/OsmPbfParser.hpp"
#include "formats/osm/CountableOsmDataVisitor.hpp"
#include "config.hpp"

#include <boost/test/unit_test.hpp>

#include <fstream>

using namespace utymap::formats;

namespace {

struct Formats_Osm_Pbf_OsmPbfParserFixture {
  Formats_Osm_Pbf_OsmPbfParserFixture() :
      istream(TEST_PBF_FILE, std::ios::binary) {
  }

  ~Formats_Osm_Pbf_OsmPbfParserFixture() {
    // In app, it should be called in class destructor which uses parser.
    google::protobuf::ShutdownProtobufLibrary();
  }

  OsmPbfParser<CountableOsmDataVisitor> parser;
  CountableOsmDataVisitor visitor;
  std::ifstream istream;
};

}

BOOST_FIXTURE_TEST_SUITE(Formats_Osm_Pbf_PbfParser, Formats_Osm_Pbf_OsmPbfParserFixture)

BOOST_AUTO_TEST_CASE(GivenDefaultPbf_WhenParserParse_ThenHasExpectedElementCount) {
  parser.parse(istream, visitor);

  BOOST_CHECK_EQUAL(visitor.nodes, 562170);
  BOOST_CHECK_EQUAL(visitor.ways, 82731);
  BOOST_CHECK_EQUAL(visitor.relations, 3064);
}

BOOST_AUTO_TEST_SUITE_END()
