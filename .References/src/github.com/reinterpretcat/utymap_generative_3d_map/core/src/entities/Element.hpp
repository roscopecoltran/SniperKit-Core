#ifndef ENTITIES_ELEMENT_HPP_DEFINED
#define ENTITIES_ELEMENT_HPP_DEFINED

#include "ElementVisitor.hpp"
#include "index/StringTable.hpp"

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>

namespace utymap {
namespace entities {

/// Represents tag
struct Tag {
  /// Returns key string id.
  std::uint32_t key;
  /// Returns value string id.
  std::uint32_t value;

  Tag() : Tag(0, 0) {}

  Tag(std::uint32_t key, std::uint32_t value) : key(key), value(value) {}

  bool operator<(const Tag &a) const { return key < a.key; }
};

/// Represents element stored in index.
struct Element {
  /// Returns id of given element.
  std::uint64_t id;
  /// Returns tag collection represented by vector of tuple<uint,uint>.
  std::vector<Tag> tags;

  virtual ~Element() = default;

  /// Visitor method.
  virtual void accept(ElementVisitor &) const = 0;

  /// Returns string representation of element.
  virtual std::string toString(utymap::index::StringTable &st) const {
    std::ostringstream stm;

    stm << '[' << id << ']' << "{";
    for (const Tag &tag : tags)
      stm << (st.getString(tag.key) + ':' + st.getString(tag.value) + ',');
    stm << "}";

    return stm.str();
  }
  // TODO prevent copy/move/assign functions for base class
};

}
}
#endif // ENTITIES_ELEMENT_HPP_DEFINED
