#ifndef ENTITIES_NODE_HPP_DEFINED
#define ENTITIES_NODE_HPP_DEFINED

#include "Element.hpp"
#include "ElementVisitor.hpp"
#include "GeoCoordinate.hpp"

namespace utymap {
namespace entities {

/// Represents point on map.
struct Node final : public Element {
  /// Returns coordinate on map.
  GeoCoordinate coordinate;

  /// Accepts visitor.
  void accept(ElementVisitor &visitor) const override {
    visitor.visitNode(*this);
  }
};

}
}

#endif // ENTITIES_NODE_HPP_DEFINED
