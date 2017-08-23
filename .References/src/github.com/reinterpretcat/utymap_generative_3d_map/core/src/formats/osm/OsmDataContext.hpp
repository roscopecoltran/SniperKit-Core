#ifndef FORMATS_OSM_OSMDATACONTEXT_HPP_DEFINED
#define FORMATS_OSM_OSMDATACONTEXT_HPP_DEFINED

#include "entities/Element.hpp"

#include <memory>
#include <unordered_map>

namespace utymap {
namespace formats {

/// Specifies visitor context which stores elements.
struct OsmDataContext {
  typedef std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Node>> NodeMapType;
  typedef std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Way>> WayMapType;
  typedef std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Area>> AreaMapType;
  typedef std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Relation>> RelationMapType;

  NodeMapType nodeMap;
  WayMapType wayMap;
  AreaMapType areaMap;
  RelationMapType relationMap;
};

}
}

#endif // FORMATS_OSM_OSMDATACONTEXT_HPP_DEFINED
