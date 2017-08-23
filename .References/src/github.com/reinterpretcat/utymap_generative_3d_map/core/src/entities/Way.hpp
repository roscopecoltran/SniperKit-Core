#ifndef ENTITIES_WAY_HPP_DEFINED
#define ENTITIES_WAY_HPP_DEFINED

#include "Element.hpp"
#include "ElementVisitor.hpp"
#include "GeoCoordinate.hpp"

#include <vector>

namespace utymap {
namespace entities {

/// Represents open polyline on map.
struct Way final : public Element {
  /// Returns way's coordinates on map.
  std::vector<GeoCoordinate> coordinates;

  /// Accepts visitor.
  void accept(ElementVisitor &visitor) const override {
    visitor.visitWay(*this);
  }
};

}
}

#endif // ENTITIES_WAY_HPP_DEFINED
