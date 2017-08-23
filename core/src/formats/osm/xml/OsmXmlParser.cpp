#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/fusion/include/io.hpp>
#include <boost/bind.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/boost_tuple.hpp>

#include "formats/FormatTypes.hpp"
#include "formats/osm/xml/OsmXmlParser.hpp"

using namespace utymap;
using namespace utymap::formats;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

BOOST_FUSION_ADAPT_STRUCT(
  Tag,
  (std::string, key)
  (std::string, value)
)

BOOST_FUSION_ADAPT_STRUCT(
  RelationMember,
  (std::string, type)
  (std::uint64_t, refId)
  (std::string, role)
)

namespace {
typedef boost::tuple<double, double, double, double> BoundsType;
typedef Tag TagType;
typedef boost::tuple<std::uint64_t, double, double, std::vector<TagType>> NodeType;
typedef boost::tuple<std::uint64_t, std::vector<std::uint64_t>, std::vector<TagType>> WayType;
typedef RelationMember MemberType;
typedef boost::tuple<std::uint64_t, std::vector<MemberType>, std::vector<TagType>> RelationType;

template <typename Iterator>
struct AttributeSkipper : qi::grammar <Iterator> {
  AttributeSkipper() : AttributeSkipper::base_type(attributeSkipper, "attr_skipper") {
      knownKey =
        ascii::string("id=") |
        ascii::string("lat=") |
        ascii::string("lon=")
          ;

      attributeSkipper =
        ascii::space |
        ((+qi::alnum - knownKey) >> "=\"" >> +(qi::char_ - '"') >> '"');
      ;

      BOOST_SPIRIT_DEBUG_NODE(knownKey);
      BOOST_SPIRIT_DEBUG_NODE(attributeSkipper);
  }

  qi::rule<Iterator> knownKey;
  qi::rule<Iterator> attributeSkipper;
};

template <typename Iterator>
struct StringValueGrammar : qi::grammar < Iterator, std::string()> {
  StringValueGrammar() : StringValueGrammar::base_type(stringValue, "string") {
    stringValue =
        '"' > *(qi::char_ - '"') > '"';

    BOOST_SPIRIT_DEBUG_NODE(stringValue);
  }

  qi::rule<Iterator, std::string()> stringValue;
};

template <typename Iterator>
struct DoubleValueGrammar : qi::grammar < Iterator, double()> {
  DoubleValueGrammar() : DoubleValueGrammar::base_type(doubleValue, "double") {
    doubleValue =
        '"' > qi::double_ > '"';

    BOOST_SPIRIT_DEBUG_NODE(doubleValue);
  }

  qi::rule<Iterator, double()> doubleValue;
};

template <typename Iterator>
struct IdGrammar : qi::grammar < Iterator, std::uint64_t()> {
  IdGrammar() : IdGrammar::base_type(id, "id") {
    id =
        qi::skip(qi::space)["id=\"" > qi::ulong_long > '"'];

    BOOST_SPIRIT_DEBUG_NODE(id);
  }

  qi::rule<Iterator, std::uint64_t()> id;
};

template <typename Iterator>
struct RefGrammar : qi::grammar < Iterator, std::uint64_t()> {
  RefGrammar() : RefGrammar::base_type(ref, "ref") {
    ref =
        qi::skip(qi::space)["ref=\"" > qi::ulong_long > '"'];

    BOOST_SPIRIT_DEBUG_NODE(ref);
  }

  qi::rule<Iterator, std::uint64_t()> ref;
};

template <typename Iterator>
struct BoundsGrammar : qi::grammar < Iterator, BoundsType()> {
  BoundsGrammar() : BoundsGrammar::base_type(bounds, "bounds") {
    bounds =
        qi::skip(qi::space)[("<bounds" > (("minlat=" > value) ^ ("minlon=" > value) ^ ("maxlat=" > value) ^ ("maxlon=" > value)) > "/>")]
        ;

    BOOST_SPIRIT_DEBUG_NODE(bounds);
  }

  DoubleValueGrammar<Iterator> value;
  qi::rule<Iterator, BoundsType()> bounds;
};

template <typename Iterator>
struct TagGrammar : qi::grammar < Iterator, TagType(), AttributeSkipper<Iterator>> {
  TagGrammar() : TagGrammar::base_type(tag, "tag") {
    tag =
        qi::skip(qi::space)["<tag" > ("k=" > value) > ("v=" > value) > "/>"]
        ;

    BOOST_SPIRIT_DEBUG_NODE(tag);
  }

  StringValueGrammar<Iterator> value;
  qi::rule<Iterator, TagType(), AttributeSkipper<Iterator>> tag;
};

template <typename Iterator>
struct NodeGrammar : qi::grammar < Iterator, NodeType(), AttributeSkipper<Iterator> > {
  NodeGrammar() : NodeGrammar::base_type(node, "node") {
    lat = qi::skip(qi::space)["lat=" > doubleValue];
    lon = qi::skip(qi::space)["lon=" > doubleValue];

    node =
        "<node" > id > lat > lon >
        (
          ('>' > *tag > "</node>") | (qi::eps > "/>")
        );

    BOOST_SPIRIT_DEBUG_NODE(lat);
    BOOST_SPIRIT_DEBUG_NODE(lon);
    BOOST_SPIRIT_DEBUG_NODE(node);
  }

  TagGrammar<Iterator> tag;
  IdGrammar<Iterator> id;
  qi::rule<Iterator, double()> lat;
  qi::rule<Iterator, double()> lon;

  DoubleValueGrammar<Iterator> doubleValue;
  qi::rule<Iterator, NodeType(), AttributeSkipper<Iterator>> node;
};

template <typename Iterator>
struct WayGrammar : qi::grammar < Iterator, WayType(), AttributeSkipper<Iterator> > {
  WayGrammar() : WayGrammar::base_type(way, "way") {
    nodeRef = qi::skip(qi::space)["<nd" > ref > "/>"];

    way =
        "<way" > id > '>' > +nodeRef > *tag > "</way>";

    BOOST_SPIRIT_DEBUG_NODE(nodeRef);
    BOOST_SPIRIT_DEBUG_NODE(way);
  }

  IdGrammar<Iterator> id;
  TagGrammar<Iterator> tag;
  RefGrammar<Iterator> ref;
  qi::rule<Iterator, std::uint64_t(), AttributeSkipper<Iterator>> nodeRef;
  qi::rule<Iterator, WayType(), AttributeSkipper<Iterator>> way;
};

template <typename Iterator>
struct RelationGrammar : qi::grammar < Iterator, RelationType(), AttributeSkipper<Iterator> > {
  RelationGrammar() : RelationGrammar::base_type(relation, "relation") {
    memberRef = qi::skip(qi::space)["<member" > ("type=" > value) > ref > ("role=" > value) > "/>"];

    relation =
        "<relation" > id > '>' > +memberRef > *tag > "</relation>";

    BOOST_SPIRIT_DEBUG_NODE(memberRef);
    BOOST_SPIRIT_DEBUG_NODE(relation);
  }

  IdGrammar<Iterator> id;
  TagGrammar<Iterator> tag;
  RefGrammar<Iterator> ref;
  StringValueGrammar<Iterator> value;
  qi::rule<Iterator, MemberType(), AttributeSkipper<Iterator>> memberRef;
  qi::rule<Iterator, RelationType(), AttributeSkipper<Iterator>> relation;
};

template <typename Visitor, typename Iterator>
struct OsmGrammar : qi::grammar < Iterator, int(), AttributeSkipper<Iterator> > {
  OsmGrammar(Visitor& visitor) : OsmGrammar::base_type(osm, "osm"), visitor(visitor) {
    osm =
      ascii::string("<?") > *(qi::char_ - '>') > '>' >
      "<osm" > '>' >
          -(qi::lexeme[+(qi::char_ - (ascii::string("<node") | "<bounds"))]) >
          -qi::omit[bounds[boost::bind(&OsmGrammar::onBounds, this, _1)]] >
          *qi::omit[node[boost::bind(&OsmGrammar::onNode, this, _1)]] >
          *qi::omit[way[boost::bind(&OsmGrammar::onWay, this, _1)]] >
          *qi::omit[relation[boost::bind(&OsmGrammar::onRelation, this, _1)]] >
      "</osm>" > qi::eps;

    BOOST_SPIRIT_DEBUG_NODE(osm);
    qi::on_error<qi::fail>(
      osm,
      error
      << phoenix::val("Error! Expecting ")
      << qi::_4
      << phoenix::val(" here: \"")
      << phoenix::construct<std::string>(qi::_3, qi::_2)
      << phoenix::val("\"")
      << std::endl);
  }

  Visitor& visitor;
  std::stringstream error;
  BoundsGrammar<Iterator> bounds;
  NodeGrammar<Iterator> node;
  WayGrammar<Iterator> way;
  RelationGrammar<Iterator> relation;
  qi::rule<Iterator, int(), AttributeSkipper<Iterator>> osm;

private:
  void onBounds(BoundsType& bounds) {
    utymap::BoundingBox boundingBox(
        utymap::GeoCoordinate(boost::get<0>(bounds), boost::get<1>(bounds)),
        utymap::GeoCoordinate(boost::get<2>(bounds), boost::get<3>(bounds)));
    visitor.visitBounds(boundingBox);
  }

  void onNode(NodeType& node) {
    utymap::GeoCoordinate coordinate(boost::get<1>(node), boost::get<2>(node));
    Tags& tags = boost::get<3>(node);
    visitor.visitNode(boost::get<0>(node), coordinate, tags);
  }

  void onWay(WayType& way) {
    auto& ids = boost::get<1>(way);
    Tags& tags = boost::get<2>(way);
    visitor.visitWay(boost::get<0>(way), ids, tags);
  }

  void onRelation(RelationType& relation) {
    auto& members = boost::get<1>(relation);
    for (auto& member : members) {
        member.type = mapType(member.type);
    }
    Tags& tags = boost::get<2>(relation);
    visitor.visitRelation(boost::get<0>(relation), members, tags);
  }

  static std::string mapType(const std::string& type) {
    if (type == "node")
        return "n";
    if (type == "way")
        return "w";
    return "r";
  }
};
}

namespace utymap { namespace formats {

template <typename Visitor>
void OsmXmlParser<Visitor>::parse(std::istream& istream, Visitor& visitor) {
  boost::spirit::istream_iterator begin(istream);
  boost::spirit::istream_iterator end;

  OsmGrammar<Visitor, boost::spirit::istream_iterator> grammar(visitor);
  AttributeSkipper<boost::spirit::istream_iterator> skipper;
  int result;

  if (!phrase_parse(begin, end, grammar, skipper, result))
      throw std::domain_error(grammar.error.str());
}

template class OsmXmlParser<OsmDataVisitor>;
template class OsmXmlParser<CountableOsmDataVisitor>;

}
}
