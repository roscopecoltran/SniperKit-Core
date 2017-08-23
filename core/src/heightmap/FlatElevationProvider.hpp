#ifndef HEIGHTMAP_FLATELEVATIONPROVIDER_HPP_DEFINED
#define HEIGHTMAP_FLATELEVATIONPROVIDER_HPP_DEFINED

#include "heightmap/ElevationProvider.hpp"

namespace utymap {
namespace heightmap {

/// Simple implementation of ElevationProvider which returns zero for all places.
class FlatElevationProvider : public ElevationProvider {
 public:

  double getElevation(const utymap::QuadKey &, const utymap::GeoCoordinate &) const override {
    return 0;
  }

  double getElevation(const utymap::QuadKey &, double, double) const override {
    return 0;
  };
};

}
}

#endif // HEIGHTMAP_FLATELEVATIONPROVIDER_HPP_DEFINED
