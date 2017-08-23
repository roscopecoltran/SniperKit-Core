#ifndef FORMATS_OSM_RELATIONPROCESSOR_HPP_DEFINED
#define FORMATS_OSM_RELATIONPROCESSOR_HPP_DEFINED

#include "formats/FormatTypes.hpp"
#include "formats/osm/OsmDataContext.hpp"
#include "utils/ElementUtils.hpp"

#include <cstdint>
#include <functional>
#include <memory>

namespace utymap {
namespace formats {

class RelationProcessor final {
 public:

  RelationProcessor(utymap::entities::Relation &relation,
                    const utymap::formats::RelationMembers &members,
                    utymap::formats::OsmDataContext &context,
                    std::unordered_map<std::uint64_t, utymap::formats::RelationMembers> &relationMembersMap,
                    std::function<void(utymap::entities::Relation &)> resolve)
      : relation_(relation), members_(members), context_(context),
        relationMembersMap_(relationMembersMap), resolve_(resolve) {
  }

  void process() {
    utymap::utils::visitRelationMembers(context_, members_, *this);
  }

  void visit(OsmDataContext::NodeMapType::const_iterator node, const std::string &role) {
    if (!isAlreadyProcessed(node->first))
      relation_.elements.push_back(node->second);
  }

  void visit(OsmDataContext::WayMapType::const_iterator way, const std::string &role) {
    if (!isAlreadyProcessed(way->first))
      relation_.elements.push_back(way->second);
  }

  void visit(OsmDataContext::AreaMapType::const_iterator area, const std::string &role) {
    if (!isAlreadyProcessed(area->first))
      relation_.elements.push_back(area->second);
  }

  void visit(OsmDataContext::RelationMapType::const_iterator rel, const std::string &role) {
    if (isAlreadyProcessed(rel->first) || hasReferenceToParent(*rel->second))
      return;

    resolve_(*rel->second);
    relation_.elements.push_back(rel->second);
  }

 private:

  bool isAlreadyProcessed(std::uint64_t id) {
    auto it = std::find_if(relation_.elements.begin(), relation_.elements.end(),
                           [&id](const std::shared_ptr<utymap::entities::Element> &e) {
                             return e->id==id;
                           });

    return it!=relation_.elements.end();
  }

  /// Checks whether relation has reference to current. If yes, it should not be
  /// processed because of recursion.
  /// NOTE This check finds only simple cases.
  bool hasReferenceToParent(const utymap::entities::Relation &rel) {
    auto parentId = relation_.id;

    auto members = relationMembersMap_.find(rel.id)->second;
    auto it = std::find_if(members.begin(), members.end(),
                           [&parentId](const utymap::formats::RelationMember &m) {
                             return m.refId==parentId;
                           });

    return it!=members.end();
  }

  utymap::entities::Relation &relation_;
  const utymap::formats::RelationMembers &members_;
  utymap::formats::OsmDataContext &context_;

  std::unordered_map<std::uint64_t, utymap::formats::RelationMembers> &relationMembersMap_;

  std::function<void(utymap::entities::Relation &)> resolve_;
};

}
}

#endif // FORMATS_OSM_RELATIONPROCESSOR_HPP_DEFINED
