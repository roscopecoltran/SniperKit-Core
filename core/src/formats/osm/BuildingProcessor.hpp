#ifndef FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED
#define FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED

#include "entities/Element.hpp"
#include "formats/FormatTypes.hpp"
#include "formats/osm/OsmDataContext.hpp"
#include "utils/ElementUtils.hpp"

#include <functional>
#include <memory>
#include <unordered_map>

namespace utymap {
namespace formats {

/// Provides the way to process buildings parts as single mesh.
class BuildingProcessor {
 public:
  BuildingProcessor(utymap::entities::Relation &relation,
                    const utymap::formats::RelationMembers &members,
                    utymap::formats::OsmDataContext &context,
                    std::function<void(utymap::entities::Relation &)> resolve)
      : relation_(relation), members_(members), context_(context), resolve_(resolve) {
  }

  void process() {
    utymap::utils::visitRelationMembers(context_, members_, *this);
  }

  void visit(OsmDataContext::NodeMapType::const_iterator node, const std::string &role) {
    addToRelation(node->second, role);
  }

  void visit(OsmDataContext::WayMapType::const_iterator way, const std::string &role) {
    addToRelation(way->second, role);
  }

  void visit(OsmDataContext::AreaMapType::const_iterator area, const std::string &role) {
    addToRelation(area->second, role);
  }

  void visit(OsmDataContext::RelationMapType::const_iterator rel, const std::string &role) {
    resolve_(*rel->second);
    addToRelation(rel->second, role);
  }

 private:

  void addToRelation(std::shared_ptr<utymap::entities::Element> element, const std::string &role) const {
    // NOTE Outline is ignored as it SHOULD BE used only for 2D renders.
    if (role!="outline")
      relation_.elements.push_back(element);

    //NOTE add specific tag to signalize that this element should not be processed separetely by building builder
    element->tags.push_back(utymap::entities::Tag(std::numeric_limits<std::uint32_t>::max(), 0));
  }

  utymap::entities::Relation &relation_;
  const utymap::formats::RelationMembers &members_;
  utymap::formats::OsmDataContext &context_;
  std::function<void(utymap::entities::Relation &)> resolve_;
};

}
}

#endif // FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED
