#ifndef GEOCOORDINATE_HPP_DEFINED
#define GEOCOORDINATE_HPP_DEFINED

#include <cmath>
#include <limits>

namespace utymap {

/// Represents geocoordinate.
struct GeoCoordinate final {
  /// Latitude.
  double latitude;
  /// Longitude.
  double longitude;

  GeoCoordinate() :
      latitude(std::numeric_limits<double>::infinity()),
      longitude(std::numeric_limits<double>::infinity()) {
  }

  GeoCoordinate(double latitude, double longitude) :
      latitude(latitude),
      longitude(longitude) {
  }

  bool operator==(const GeoCoordinate &rhs) const {
    return std::abs(latitude - rhs.latitude) < std::numeric_limits<double>::epsilon() &&
        std::abs(longitude - rhs.longitude) < std::numeric_limits<double>::epsilon();
  }

  bool isValid() const {
    // NOTE please note that lowest and biggest values are not included.
    return latitude > -90 && latitude < 90 && longitude > -180 && longitude < 180;
  }
};

}

#endif // GEOCOORDINATE_HPP_DEFINED
