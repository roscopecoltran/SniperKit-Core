#ifndef ENTITIES_AREA_HPP_DEFINED
#define ENTITIES_AREA_HPP_DEFINED

#include "Element.hpp"
#include "ElementVisitor.hpp"
#include "GeoCoordinate.hpp"

#include <vector>

namespace utymap {
namespace entities {

/// Represents closed polyline on map.
struct Area final : public Element {
  /// Returns way's coordinates on map.
  std::vector<GeoCoordinate> coordinates;

  /// Accepts visitor.
  void accept(ElementVisitor &visitor) const override {
    visitor.visitArea(*this);
  }
};

}
}

#endif // ENTITIES_AREA_HPP_DEFINED
