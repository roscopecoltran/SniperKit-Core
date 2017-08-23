#ifndef UTILS_ELEMENTUTILS_HPP_DEFINED
#define UTILS_ELEMENTUTILS_HPP_DEFINED

#include "entities/Element.hpp"
#include "formats/FormatTypes.hpp"
#include "formats/osm/OsmDataContext.hpp"
#include "index/StringTable.hpp"
#include "utils/CoreUtils.hpp"

#include <algorithm>

namespace utymap {
namespace utils {

/// Sets tags to element.
inline void setTags(const utymap::index::StringTable &stringTable,
                    utymap::entities::Element &element,
                    const utymap::formats::Tags &tags) {
  for (const auto &tag : tags) {
    std::uint32_t key = stringTable.getId(tag.key);
    std::uint32_t value = stringTable.getId(tag.value);
    element.tags.push_back(utymap::entities::Tag(key, value));
  }
  // NOTE: tags should be sorted to speed up mapcss styling
  std::sort(element.tags.begin(), element.tags.end());
}

/// Convert format specific tags to entity ones.
inline std::vector<utymap::entities::Tag> convertTags(const utymap::index::StringTable &stringTable,
                                                      const utymap::formats::Tags &tags) {
  std::vector<utymap::entities::Tag> convertedTags(tags.size());
  std::transform(tags.begin(), tags.end(), convertedTags.begin(), [&](const utymap::formats::Tag &tag) {
    return utymap::entities::Tag(stringTable.getId(tag.key), stringTable.getId(tag.value));
  });

  std::sort(convertedTags.begin(), convertedTags.end());

  return std::move(convertedTags);
}

template<typename T>
std::uint32_t getTagValue(std::uint32_t key,
                          const std::vector<utymap::entities::Tag> &tags,
                          std::uint32_t defaultValue,
                          const T &t) {
  auto begin = tags.begin();
  auto end = tags.end();
  while (begin < end) {
    const auto middle = begin + (std::distance(begin, end)/2);
    if (middle->key==key)
      return t(middle->value);

    if (middle->key > key)
      end = middle;
    else
      begin = middle + 1;
  }

  return defaultValue;
}

inline bool hasTag(std::uint32_t key,
                   std::uint32_t value,
                   const std::vector<utymap::entities::Tag> &tags) {
  return value==getTagValue(
      key, tags,
      std::numeric_limits<std::uint32_t>::max(),
      [](const std::uint32_t v) {
        return v;
      });
}

inline bool hasIgnore(const std::vector<utymap::entities::Tag> &tags) {
  static const std::uint32_t IgnoreKey = std::numeric_limits<std::uint32_t>::max();
  static const std::uint32_t IgnoreValue = 0;
  return hasTag(IgnoreKey, IgnoreValue, tags);
}

inline std::string getTagValue(std::uint32_t key,
                               const std::vector<utymap::entities::Tag> &tags,
                               const utymap::index::StringTable &stringTable) {
  return stringTable.getString(getTagValue(
      key, tags,
      std::numeric_limits<std::uint32_t>::max(),
      [&](const std::uint32_t v) {
        if (v==std::numeric_limits<std::uint32_t>::max())
          throw std::domain_error("Cannot find tag:" + stringTable.getString(key));
        return v;
      }));
}

///Gets mesh name
inline std::string getMeshName(const std::string &prefix, const utymap::entities::Element &element) {
  return prefix + utymap::utils::toString(element.id);
}

template<typename T>
static void visitRelationMembers(const utymap::formats::OsmDataContext &context,
                                 const utymap::formats::RelationMembers &members,
                                 T &visitor) {
  for (const auto &member : members) {
    if (member.type=="n") {
      auto nodePair = context.nodeMap.find(member.refId);
      if (nodePair!=context.nodeMap.end())
        visitor.visit(nodePair, member.role);
    } else if (member.type=="w") {
      auto areaPair = context.areaMap.find(member.refId);
      if (areaPair!=context.areaMap.end()) {
        visitor.visit(areaPair, member.role);
      } else {
        auto wayPair = context.wayMap.find(member.refId);
        if (wayPair!=context.wayMap.end())
          visitor.visit(wayPair, member.role);
      }
    } else {
      auto relationPair = context.relationMap.find(member.refId);
      if (relationPair!=context.relationMap.end())
        visitor.visit(relationPair, member.role);
    }
  }
}

}
}

#endif // UTILS_ELEMENTUTILS_HPP_DEFINED
