#ifndef FORMATS_OSM_OSMDATAVISITOR_HPP_DEFINED
#define FORMATS_OSM_OSMDATAVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "formats/FormatTypes.hpp"
#include "formats/osm/OsmDataContext.hpp"
#include "index/StringTable.hpp"

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

namespace utymap {
namespace formats {

class OsmDataVisitor final {
 public:

  OsmDataVisitor(const utymap::index::StringTable &stringTable,
                 std::function<bool(utymap::entities::Element &)> add);

  void visitBounds(utymap::BoundingBox bbox);

  void visitNode(std::uint64_t id, utymap::GeoCoordinate &coordinate, utymap::formats::Tags &tags);

  void visitWay(std::uint64_t id, std::vector<std::uint64_t> &nodeIds, utymap::formats::Tags &tags);

  void visitRelation(std::uint64_t id, utymap::formats::RelationMembers &members, utymap::formats::Tags &tags);

  void add(utymap::entities::Element &element);

  void complete();

 private:

  bool hasTag(const std::string &key, const std::string &value, const std::vector<utymap::entities::Tag> &tags) const;
  void resolve(utymap::entities::Relation &relation);

  const utymap::index::StringTable &stringTable_;
  std::function<bool(utymap::entities::Element &)> add_;
  utymap::formats::OsmDataContext context_;
  std::unordered_map<std::uint64_t, utymap::formats::RelationMembers> relationMembers_;
};

}
}

#endif // FORMATS_OSM_OSMDATAVISITOR_HPP_DEFINED
