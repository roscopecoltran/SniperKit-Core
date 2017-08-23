#ifndef FORMATS_XML_OSMXMLPARSER_HPP_INCLUDED
#define FORMATS_XML_OSMXMLPARSER_HPP_INCLUDED

#include "formats/osm/OsmDataVisitor.hpp"
#include "formats/osm/CountableOsmDataVisitor.hpp"

namespace utymap {
namespace formats {

template<typename Visitor>
class OsmXmlParser {
 public:
  /// Parses osm xml data from stream calling visitor.
  void parse(std::istream &istream, Visitor &visitor);
};
}
}

#endif  // FORMATS_XML_OSMXMLPARSER_HPP_INCLUDED
