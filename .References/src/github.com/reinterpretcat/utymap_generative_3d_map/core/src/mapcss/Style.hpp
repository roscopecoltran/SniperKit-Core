#ifndef MAPCSS_STYLE_HPP_INCLUDED
#define MAPCSS_STYLE_HPP_INCLUDED

#include "Exceptions.hpp"
#include "entities/Element.hpp"
#include "mapcss/StyleConsts.hpp"
#include "mapcss/StyleDeclaration.hpp"
#include "index/StringTable.hpp"
#include "utils/CoreUtils.hpp"
#include "utils/GeoUtils.hpp"

#include <cstdint>
#include <string>
#include <set>
#include <unordered_map>

namespace utymap {
namespace mapcss {

/// Represents style for element.
struct Style final {
  Style(const std::vector<utymap::entities::Tag> &tags,
        const utymap::index::StringTable &stringTable) :
      stringTable_(stringTable), builderKeyId_(stringTable.getId(StyleConsts::BuilderKey())),
      tags_(tags), declarations_(), builders_() {
  }

  Style(Style &&other) :
      stringTable_(other.stringTable_),
      builderKeyId_(other.builderKeyId_),
      tags_(std::move(other.tags_)),
      declarations_(std::move(other.declarations_)),
      builders_(std::move(other.builders_)) {
  }

  Style(const Style &) = default;

  Style &operator=(const Style &) = delete;
  Style &operator=(Style &&) = delete;

  bool has(std::uint32_t key) const {
    return declarations_.find(key)!=declarations_.end();
  }

  bool has(std::uint32_t key, const std::string &value) const {
    auto it = declarations_.find(key);
    return it!=declarations_.end() && it->second->value()==value;
  }

  bool empty() const {
    return declarations_.empty();
  }

  void put(const StyleDeclaration &declaration) {
    if (declaration.key()==builderKeyId_) {
      builders_.insert(declaration.value());
    }
    // TODO remove this branch if condition above is true, after style migration
    declarations_[declaration.key()] = &declaration;
  }

  const StyleDeclaration &get(std::uint32_t key) const {
    auto it = declarations_.find(key);
    if (it==declarations_.end())
      throw MapCssException(std::string("Cannot find declaration with the key: ") + stringTable_.getString(key));

    return *it->second;
  }

  std::vector<const StyleDeclaration *> declarations() const {
    std::vector<const StyleDeclaration *> decs;
    std::transform(std::begin(declarations_), std::end(declarations_), std::back_inserter(decs),
                   [](std::unordered_map<std::uint32_t, const StyleDeclaration *>::value_type const &pair) {
                     return pair.second;
                   });

    return decs;
  }

  /// Gets list of builders extracted from declarations.
  std::vector<std::string> getBuilders() const {
    std::vector<std::string> builders;
    std::transform(builders_.cbegin(), builders_.cend(), std::back_inserter(builders),
      [](std::string const &item) {
      return item;
    });

    return builders;
  }

  /// Gets string by given key. Empty string by default
  std::string getString(const std::string &key) const {
    std::uint32_t keyId = stringTable_.getId(key);
    return getString(keyId);
  }

  /// Gets string by given key. Empty string by default
  std::string getString(std::uint32_t keyId) const {
    if (!has(keyId))
      return "";

    auto &declaration = get(keyId);

    return declaration.isEval()
           ? declaration.evaluate<std::string>(tags_, stringTable_)
           : declaration.value();
  }

  /// Gets double value or zero.
  double getValue(const std::string &key) const {
    return getValue(key, 1);
  }

  /// Gets double value or zero.
  /// Relative size is used when dimension is specified
  double getValue(const std::string &key, double relativeSize) const {
    return getValue(key, relativeSize, BoundingBox());
  }

  /// Gets double value or zero.
  /// Bounding box is used when dimension is specified
  double getValue(const std::string &key, const BoundingBox &bbox) const {
    return getValue(key, bbox.height(), bbox);
  }

 private:

  /// Gets double value or zero.
  /// Bounding box is used when dimension is specified
  double getValue(const std::string &key, double relativeSize, const BoundingBox &bbox) const {
    std::uint32_t keyId = stringTable_.getId(key);

    if (!has(keyId))
      return 0;

    const auto &declaration = get(keyId);
    const auto &rawValue = declaration.value();
    char dimen = rawValue[rawValue.size() - 1];

    if (dimen=='m') {
      double value = utymap::utils::parseDouble(rawValue.substr(0, rawValue.size() - 1));
      return bbox.isValid()
             ? utymap::utils::GeoUtils::getOffset(bbox.center(), value)
             : value;
    }

    if (dimen=='%') {
      double value = utymap::utils::parseDouble(rawValue.substr(0, rawValue.size() - 1));
      return relativeSize*value*0.01;
    }

    return declaration.isEval()
           ? declaration.evaluate<double>(tags_, stringTable_)
           : utymap::utils::parseDouble(rawValue);
  }

  const utymap::index::StringTable &stringTable_;
  const std::uint64_t builderKeyId_;
  std::vector<utymap::entities::Tag> tags_;
  std::unordered_map<std::uint32_t, const StyleDeclaration *> declarations_;
  std::set<std::string> builders_;
};

}
}
#endif  // MAPCSS_STYLE_HPP_INCLUDED
