#ifndef MAPCSS_STYLEDECLARATION_HPP_INCLUDED
#define MAPCSS_STYLEDECLARATION_HPP_INCLUDED

#include "Exceptions.hpp"
#include "entities/Element.hpp"
#include "index/StringTable.hpp"
#include "mapcss/StyleEvaluator.hpp"
#include "utils/ElementUtils.hpp"

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace utymap {
namespace mapcss {

/// Represents style declaration which support evaluation.
struct StyleDeclaration final {
  StyleDeclaration(std::uint32_t key, const std::string &value) :
      key_(key),
      value_(value),
      tree_(StyleEvaluator::parse(value)) {
  }

  ~StyleDeclaration() {};
  StyleDeclaration(StyleDeclaration &&other) :
      key_(other.key_), value_(other.value_), tree_(std::move(other.tree_)) {
  }

  StyleDeclaration(const StyleDeclaration &) = delete;
  StyleDeclaration &operator=(StyleDeclaration &&other) = delete;
  StyleDeclaration &operator=(const StyleDeclaration &) = delete;

  /// Gets declaration key.
  std::uint32_t key() const { return key_; };

  /// Gets declaration value.
  const std::string &value() const { return value_; };

  /// Gets true if declaration should be evaluated
  bool isEval() const { return tree_!=nullptr; }

  /// Evaluates expression using tags
  template<typename T>
  T evaluate(const std::vector<utymap::entities::Tag> &tags,
             const utymap::index::StringTable &stringTable) const {
    if (!isEval())
      throw utymap::MapCssException("Cannot evaluate raw value.");

    return StyleEvaluator::evaluate<T>(*tree_, tags, stringTable);
  }

 private:

  std::uint32_t key_;
  std::string value_;
  std::unique_ptr<StyleEvaluator::Tree> tree_;
};

}
}
#endif  // MAPCSS_STYLEDECLARATION_HPP_INCLUDED
