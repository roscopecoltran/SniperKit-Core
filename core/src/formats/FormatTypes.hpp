#ifndef FORMATS_FORMATTYPES_HPP_DEFINED
#define FORMATS_FORMATTYPES_HPP_DEFINED

#include "GeoCoordinate.hpp"

#include <vector>
#include <cstdint>
#include <string>

namespace utymap {
namespace formats {

enum class FormatType {
  Pbf = 0,
  Xml = 1,
  Shape = 2,
  Json = 3
};

struct Tag final {
  std::string key;
  std::string value;

  Tag() {
  }

  Tag(const std::string &key, const std::string &value) :
      key(std::move(key)),
      value(std::move(value)) {
  }
};

/// this type is specific for osm formats
struct RelationMember final {
  std::uint64_t refId;
  std::string type;
  std::string role;
};

/// this type is specific for shapefile format
struct PolygonMember final {
  bool isRing;
  std::vector<GeoCoordinate> coordinates;
};

typedef std::vector<Tag> Tags;
typedef std::vector<RelationMember> RelationMembers;
typedef std::vector<PolygonMember> PolygonMembers;
typedef std::vector<utymap::GeoCoordinate> Coordinates;

}
}

#endif // FORMATS_FORMATTYPES_HPP_DEFINED
