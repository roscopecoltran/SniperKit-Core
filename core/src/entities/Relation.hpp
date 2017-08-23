#ifndef ENTITIES_RELATION_HPP_DEFINED
#define ENTITIES_RELATION_HPP_DEFINED

#include "entities/Element.hpp"
#include "entities/ElementVisitor.hpp"

#include <vector>
#include <memory>

namespace utymap {
namespace entities {

/// Represents union of elements on map.
struct Relation final : public Element {
  std::vector<std::shared_ptr<Element>> elements;

  /// Accepts visitor.
  void accept(ElementVisitor &visitor) const override {
    visitor.visitRelation(*this);
  }
};

}
}

#endif // ENTITIES_RELATION_HPP_DEFINED
