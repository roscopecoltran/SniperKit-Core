#ifndef INDEX_SHAPEDATAVISITOR_HPP_DEFINED
#define INDEX_SHAPEDATAVISITOR_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/FormatTypes.hpp"
#include "index/StringTable.hpp"
#include "utils/ElementUtils.hpp"

#include <algorithm>
#include <functional>
#include <cstdint>
#include <memory>

namespace utymap {
namespace formats {

struct ShapeDataVisitor final {
  int nodes;
  int ways;
  int areas;
  int relations;

  ShapeDataVisitor(const utymap::index::StringTable &stringTable,
                   std::function<bool(utymap::entities::Element &)> functor) :
      nodes(0),
      ways(0),
      areas(0),
      relations(0),
      stringTable_(stringTable),
      functor_(functor) {
  }

  void visitNode(utymap::GeoCoordinate &coordinate, utymap::formats::Tags &tags) {
    utymap::entities::Node node;
    node.id = 0;
    node.coordinate = coordinate;
    utymap::utils::setTags(stringTable_, node, tags);
    if (functor_(node))
      nodes++;
  }

  void visitWay(utymap::formats::Coordinates &coordinates, utymap::formats::Tags &tags, bool isRing) {
    if (isRing) {
      utymap::entities::Area area;
      area.id = 0;
      area.coordinates = std::move(coordinates);
      utymap::utils::setTags(stringTable_, area, tags);
      if (functor_(area))
        areas++;
    } else {
      utymap::entities::Way way;
      way.id = 0;
      way.coordinates = std::move(coordinates);
      utymap::utils::setTags(stringTable_, way, tags);
      if (functor_(way))
        ways++;
    }
  }

  void visitRelation(utymap::formats::PolygonMembers &members, utymap::formats::Tags &tags) {
    utymap::entities::Relation relation;
    relation.id = 0;
    utymap::utils::setTags(stringTable_, relation, tags);
    for (const auto &member : members) {
      if (member.coordinates.size()==1) {
        auto node = std::make_shared<utymap::entities::Node>();
        node->id = 0;
        node->coordinate = member.coordinates[0];
        relation.elements.push_back(node);
      } else if (member.isRing) {
        auto area = std::make_shared<utymap::entities::Area>();
        area->id = 0;
        area->coordinates = std::move(member.coordinates);
        relation.elements.push_back(area);
      } else {
        auto way = std::make_shared<utymap::entities::Way>();
        way->id = 0;
        way->coordinates = std::move(member.coordinates);
        relation.elements.push_back(way);
      }
    }
    if (functor_(relation))
      relations++;
  }

  void complete() {}

 private:
  const utymap::index::StringTable &stringTable_;
  std::function<bool(utymap::entities::Element &)> functor_;
};

}
}

#endif // INDEX_SHAPEDATAVISITOR_HPP_DEFINED
