#ifndef MAPCSS_STYLESHEET_HPP_DEFINED
#define MAPCSS_STYLESHEET_HPP_DEFINED

#include "math/Rectangle.hpp"
#include "lsys/LSystem.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace utymap {
namespace mapcss {

/// Represents a single texture region inside texture atlas.
struct TextureRegion final {
  TextureRegion() :
      TextureRegion(0, 0, 0, 0, 0, 0) {
  }

  TextureRegion(std::uint16_t atlasWidth, std::uint16_t atlasHeight,
                std::uint16_t x, std::uint16_t y,
                std::uint16_t width, std::uint16_t height) :
      atlasWidth(atlasWidth), atlasHeight(atlasHeight),
      x(x), y(y),
      width(width), height(height) {
  }

  bool isEmpty() const {
    return atlasWidth==0 || atlasHeight==0;
  }

  std::uint16_t atlasWidth, atlasHeight, x, y, width, height;
};

/// Holds list of textures which represent the same material.
struct TextureGroup final {
  /// Adds specific region to the group.
  void add(std::uint16_t width, std::uint16_t height, utymap::math::Rectangle rect) {
    regions_.emplace_back(
        width,
        height,
        static_cast<std::uint16_t>(rect.xMin),
        static_cast<std::uint16_t>(rect.yMin),
        static_cast<std::uint16_t>(rect.width()),
        static_cast<std::uint16_t>(rect.height()));
  }

  /// Returns pseudo random region.
  const TextureRegion &random(std::uint64_t seed) const {
    return regions_[seed%regions_.size()];
  }

 private:
  std::vector<TextureRegion> regions_;
};

/// Represents texture atlas.
struct TextureAtlas final {
  using Groups = std::unordered_map<std::string, TextureGroup>;

  TextureAtlas() : TextureAtlas(0, Groups()) {
  }

  TextureAtlas(std::uint16_t index, const Groups &textureGroups) :
      index_(index),
      textureGroups_(std::move(textureGroups)),
      emptyGroup_() {
    emptyGroup_.add(0, 0, utymap::math::Rectangle());
  }

  /// Returns name of atlas.
  std::uint16_t index() const {
    return index_;
  }

  /// Returns a reference to texture group.
  /// Note: returns raw reference from map.
  const TextureGroup &get(const std::string &key) const {
    auto group = textureGroups_.find(key);
    if (group==textureGroups_.end()) {
      return emptyGroup_;
    }

    return group->second;
  }

 private:
  std::uint16_t index_;
  Groups textureGroups_;
  TextureGroup emptyGroup_;
};

/// Represents condition.
struct Condition final {
  std::string key;
  std::string operation;
  std::string value;
};

/// Represents map zoom level.
struct Zoom final {
  std::uint8_t start;
  std::uint8_t end;
};

/// Represents selector.
struct Selector final {
  std::vector<std::string> names;
  Zoom zoom;
  std::vector<Condition> conditions;
};

/// Represents declaration.
struct Declaration final {
  std::string key;
  std::string value;
};

/// Represents rule.
struct Rule final {
  std::vector<Selector> selectors;
  std::vector<Declaration> declarations;
};

/// Represents stylesheet.
struct StyleSheet final {
  std::vector<Rule> rules;
  std::vector<TextureAtlas> textures;
  std::unordered_map<std::string, utymap::lsys::LSystem> lsystems;
};

}
}
#endif // MAPCSS_STYLESHEET_HPP_DEFINED
