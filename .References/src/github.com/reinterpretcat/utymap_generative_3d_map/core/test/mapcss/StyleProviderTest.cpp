#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Relation.hpp"
#include "mapcss/StyleProvider.hpp"
#include "test_utils/ElementUtils.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"

using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::tests;

namespace {
struct Index_StyleProviderFixture {
  Index_StyleProviderFixture() :
      dependencyProvider(),
      stylesheet(new StyleSheet()) {
  }

  void setSingleSelector(int zoomStart, int zoomEnd,
                         const std::initializer_list<std::string> &names,
                         const std::initializer_list<utymap::mapcss::Condition> &conditions,
                         const std::initializer_list<utymap::mapcss::Declaration> &declarations = {}) {
    auto selector = Selector();
    selector.names.insert(selector.names.begin(), names.begin(), names.end());
    selector.zoom.start = static_cast<std::uint8_t>(zoomStart);
    selector.zoom.end = static_cast<std::uint8_t>(zoomEnd);
    for (const auto &condition : conditions) {
      selector.conditions.push_back(condition);
    }

    Rule rule;
    for (const auto &declaration : declarations) {
      rule.declarations.push_back(declaration);
    }

    rule.selectors.push_back(selector);
    stylesheet->rules.push_back(rule);
    styleProvider = std::make_shared<StyleProvider>(
        *stylesheet,
        *dependencyProvider.getStringTable());
  }

  DependencyProvider dependencyProvider;
  std::shared_ptr<StyleProvider> styleProvider;
  std::shared_ptr<StyleSheet> stylesheet;
};
}

BOOST_FIXTURE_TEST_SUITE(Index_StyleProvider, Index_StyleProviderFixture)

BOOST_AUTO_TEST_CASE(GivenSimpleEqualsCondition_WhenHasStyle_ThenReturnTrue) {
  int zoomLevel = 1;
  setSingleSelector(zoomLevel, zoomLevel, {"node"}, {{"amenity", "=", "biergarten"}});
  Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0,
                                                {
                                                    std::make_pair("amenity", "biergarten")
                                                });

  BOOST_CHECK(styleProvider->hasStyle(node, zoomLevel));
}

BOOST_AUTO_TEST_CASE(GivenSimpleLessCondition_WhenHasStyle_ThenReturnsExpectedLogicalResult) {
  int zoomLevel = 1;
  setSingleSelector(zoomLevel, zoomLevel, {"node"}, {{"level", "<", "0"}});

  BOOST_CHECK(styleProvider->hasStyle(
      ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0, {{"level", "-1"}}),
      zoomLevel));

  BOOST_CHECK(!styleProvider->hasStyle(
      ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0, {{"level", "1"}}),
      zoomLevel));
}

BOOST_AUTO_TEST_CASE(GivenSimpleGreaterCondition_WhenHasStyle_ThenReturnsExpectedLogicalResult) {
  int zoomLevel = 1;
  setSingleSelector(zoomLevel, zoomLevel, {"node"}, {{"level", ">", "0"}});

  BOOST_CHECK(styleProvider->hasStyle(
      ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0, {{"level", "1"}}),
      zoomLevel));

  BOOST_CHECK(!styleProvider->hasStyle(
      ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0, {{"level", "-1"}}),
      zoomLevel));
}

BOOST_AUTO_TEST_CASE(GivenTwoNamesAndSimpleEqualsCondition_WhenHasStyleForSecondName_ThenReturnTrue) {
  int zoomLevel = 1;
  setSingleSelector(zoomLevel, zoomLevel, {"way", "node"}, {{"amenity", "=", "biergarten"}});
  Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0,
                                                {
                                                    std::make_pair("amenity", "biergarten")
                                                });

  BOOST_CHECK(styleProvider->hasStyle(node, zoomLevel));
}

BOOST_AUTO_TEST_CASE(GivenSimpleEqualsConditionButDifferentZoomLevel_WhenHasStyle_ThenReturnFalse) {
  int zoomLevel = 1;
  setSingleSelector(zoomLevel, zoomLevel, {"node"}, {{"amenity", "=", "biergarten"}});
  Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0,
                                                {
                                                    std::make_pair("amenity", "biergarten")
                                                });

  BOOST_CHECK(!styleProvider->hasStyle(node, 2));
}

BOOST_AUTO_TEST_CASE(GivenTwoEqualsConditions_WhenHasStyle_ThenReturnTrue) {
  int zoomLevel = 1;
  setSingleSelector(zoomLevel, zoomLevel, {"node"},
                    {
                        {"amenity", "=", "biergarten"},
                        {"address", "=", "Invalidstr."}
                    });
  Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0,
                                                {
                                                    std::make_pair("amenity", "biergarten"),
                                                    std::make_pair("address", "Invalidstr.")
                                                });

  BOOST_CHECK(styleProvider->hasStyle(node, zoomLevel));
}

BOOST_AUTO_TEST_CASE(GivenTwoNotEqualsConditions_WhenHasStyle_ThenReturnFalse) {
  int zoomLevel = 1;
  setSingleSelector(zoomLevel, zoomLevel, {"node"},
                    {
                        {"amenity", "=", "biergarten"},
                        {"address", "!=", "Invalidstr."}
                    });
  Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0,
                                                {
                                                    std::make_pair("amenity", "biergarten"),
                                                    std::make_pair("address", "Invalidstr.")
                                                });

  BOOST_CHECK(!styleProvider->hasStyle(node, zoomLevel));
}

BOOST_AUTO_TEST_CASE(GivenCustomElementRuleOverridesDefault_WhenForElement_ThenStyleHasOnlyCustomRule) {
  int zoomLevel = 1;
  setSingleSelector(zoomLevel, zoomLevel, {"node"},
                    {{"amenity", "=", "biergarten"}},
                    {{"key1", "value1"}});
  setSingleSelector(zoomLevel, zoomLevel, {"element"},
                    {{"id", "=", "7"}},
                    {{"key2", "value2"}});
  Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 7,
                                                {std::make_pair("amenity", "biergarten")});

  Style style = styleProvider->forElement(node, zoomLevel);

  BOOST_CHECK(style.has(dependencyProvider.getStringTable()->getId("key2"), "value2"));
  BOOST_CHECK(!style.has(dependencyProvider.getStringTable()->getId("key1")));
}

BOOST_AUTO_TEST_CASE(GivenTwoDifferentStyles_WhenConstructed_ThenTheyHaveDifferentTags) {
  int zoomLevel = 1;
  setSingleSelector(zoomLevel, zoomLevel, {"node"}, {{"a", "=", "b"}}, {{"k", "v"}});
  auto tag1 = styleProvider->getTag();
  stylesheet->rules.clear();

  setSingleSelector(zoomLevel, zoomLevel, {"node"}, {{"a", "!=", "b"}}, {{"k", "v"}});
  auto tag2 = styleProvider->getTag();

  BOOST_CHECK_NE(tag1, tag2);
}

BOOST_AUTO_TEST_CASE(GivenTwoEqualStyles_WhenConstructed_ThenTheyHaveTheSameTag) {
  int zoomLevel = 1;
  setSingleSelector(zoomLevel, zoomLevel, {"node"}, {{"a", "=", "b"}}, {{"k", "v"}});
  auto tag1 = styleProvider->getTag();
  stylesheet->rules.clear();

  setSingleSelector(zoomLevel, zoomLevel, {"node"}, {{"a", "=", "b"}}, {{"k", "v"}});
  auto tag2 = styleProvider->getTag();

  BOOST_CHECK_EQUAL(tag1, tag2);
}

BOOST_AUTO_TEST_SUITE_END()
