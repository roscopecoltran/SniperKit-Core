#include "Exceptions.hpp"
#include "mapcss/TextureAtlasParser.hpp"

#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/bind.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>

using namespace utymap::mapcss;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

namespace {

struct Region {
  std::string groupName;
  int x;
  int y;
  int width;
  int height;
};

struct Atlas {
  int id;
  int width;
  int height;
  std::vector<Region> regions;
};

}

BOOST_FUSION_ADAPT_STRUCT(
  Region,
  (std::string, groupName)
  (int, x)
  (int, y)
  (int, width)
  (int, height)
)

BOOST_FUSION_ADAPT_STRUCT(
  Atlas,
  (int, id)
  (int, width)
  (int, height)
  (std::vector<Region>, regions)
)

template<typename Iterator>
struct RegionGrammar : qi::grammar<Iterator, Region(), qi::space_type> {
  RegionGrammar() : RegionGrammar::base_type(start, "region") {
    start =
      qi::lexeme[+(ascii::char_ - ',')] >> ',' >>
      qi::int_ >> ',' >>
      qi::int_ >> ',' >>
      qi::int_ >> ',' >>
      qi::int_;

    start.name("region");
  }
  qi::rule<Iterator, Region(), qi::space_type> start;
};

template<typename Iterator>
struct AtlasGrammar : qi::grammar<Iterator, Atlas(), qi::space_type> {
  AtlasGrammar() : AtlasGrammar::base_type(start, "atlas") {
    start =
      qi::int_ >> ',' >>
      qi::int_ >> ',' >>
      qi::int_ >>
      +(region);
  }

  RegionGrammar<Iterator> region;
  qi::rule<Iterator, Atlas(), qi::space_type> start;
};

template<typename Iterator>
TextureAtlas parse(Iterator begin, Iterator end) {
  Atlas atlas;
  AtlasGrammar<Iterator> grammar;

  if (!phrase_parse(begin, end, grammar, qi::space, atlas))
    throw utymap::MapCssException(std::string("Cannot parse texture atlas."));

  std::unordered_map<std::string, TextureGroup> groups;
  for (const auto &region : atlas.regions) {
    groups[region.groupName].add(
        static_cast<std::uint16_t>(atlas.width),
        static_cast<std::uint16_t>(atlas.height),
        utymap::math::Rectangle(region.x,
                                region.y,
                                region.x + region.width,
                                region.y + region.height));
  }

  return TextureAtlas(static_cast<std::uint16_t>(atlas.id), groups);
}

TextureAtlas TextureAtlasParser::parse(const std::string &content) const {
  return ::parse(content.begin(), content.end());
}

TextureAtlas TextureAtlasParser::parse(std::istream &istream) const {
  std::string content((std::istreambuf_iterator<char>(istream)), std::istreambuf_iterator<char>());
  return parse(content);
}
