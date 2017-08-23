#include "index/StringTable.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"

using namespace utymap::index;
using namespace utymap::tests;

namespace {
struct Index_StringTableFixture {
  DependencyProvider dependencyProvider;
};
}

BOOST_FIXTURE_TEST_SUITE(Index_StringTable, Index_StringTableFixture)

BOOST_AUTO_TEST_CASE(GivenNonPresentString_WhenGetIdFirstTime_ThenReturnZero) {
  uint32_t id = dependencyProvider.getStringTable()->getId("some_string");

  BOOST_CHECK_EQUAL(id, 0);
}

BOOST_AUTO_TEST_CASE(GivenTable_WhenInsertMultiple_ThenReturnSequentialId) {
  uint32_t id1 = dependencyProvider.getStringTable()->getId("string1");
  uint32_t id2 = dependencyProvider.getStringTable()->getId("string2");
  uint32_t id3 = dependencyProvider.getStringTable()->getId("string3");

  BOOST_CHECK_EQUAL(id1, 0);
  BOOST_CHECK_EQUAL(id2, 1);
  BOOST_CHECK_EQUAL(id3, 2);
}

BOOST_AUTO_TEST_CASE(GivenThreeStrings_WhenGetIdOfSecond_ThenReturnValidId) {
  dependencyProvider.getStringTable()->getId("string1");
  dependencyProvider.getStringTable()->getId("string2");
  dependencyProvider.getStringTable()->getId("string3");

  std::uint32_t id = dependencyProvider.getStringTable()->getId("string2");

  BOOST_CHECK_EQUAL(id, 1);
}

BOOST_AUTO_TEST_CASE(GivenThreeStrings_WhenGetStringOfSecond_ThenReturnValidString) {
  dependencyProvider.getStringTable()->getId("string1");
  dependencyProvider.getStringTable()->getId("string2");
  dependencyProvider.getStringTable()->getId("string3");

  std::string str = dependencyProvider.getStringTable()->getString(1);

  BOOST_CHECK_EQUAL(str, "string2");
}

BOOST_AUTO_TEST_SUITE_END()
