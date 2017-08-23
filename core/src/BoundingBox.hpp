#ifndef BOUNDINGBOX_HPP_DEFINED
#define BOUNDINGBOX_HPP_DEFINED

#include "GeoCoordinate.hpp"

#include <algorithm>

namespace utymap {

/// Represents geo bounding box
struct BoundingBox final {
  /// Point with minimal latitude and longitude.
  GeoCoordinate minPoint;
  /// Point with maximum latitude and longitude.
  GeoCoordinate maxPoint;

  BoundingBox() :
      BoundingBox(GeoCoordinate(90, 180), GeoCoordinate(-90, -180)) {
  }

  BoundingBox(const GeoCoordinate &minPoint, const GeoCoordinate &maxPoint) :
      minPoint(minPoint), maxPoint(maxPoint) {
  }

  BoundingBox &operator+=(const BoundingBox &rhs) {
    expand(rhs);
    return *this;
  }

  bool isValid() const {
    // TODO possible that minLon > maxLon at some locations
    return minPoint.latitude <= maxPoint.latitude &&
        minPoint.longitude <= maxPoint.longitude;
  }

  /// Expands bounding box using another bounding box.
  void expand(const BoundingBox &rhs) {
    minPoint.latitude = std::min(minPoint.latitude, rhs.minPoint.latitude);
    minPoint.longitude = std::min(minPoint.longitude, rhs.minPoint.longitude);

    maxPoint.latitude = std::max(maxPoint.latitude, rhs.maxPoint.latitude);
    maxPoint.longitude = std::max(maxPoint.longitude, rhs.maxPoint.longitude);
  }

  /// Expands bounding box using given coordinate.
  void expand(const GeoCoordinate &c) {
    minPoint = GeoCoordinate(
        std::min(minPoint.latitude, c.latitude),
        std::min(minPoint.longitude, c.longitude));

    maxPoint = GeoCoordinate(
        std::max(maxPoint.latitude, c.latitude),
        std::max(maxPoint.longitude, c.longitude));
  }

  /// Expands bounging box from collection of geo data.
  template<typename ForwardIterator>
  void expand(ForwardIterator begin, ForwardIterator end) {
    for (; begin!=end; ++begin)
      expand(*begin);
  }

  /// Checks whether given bounding box inside the current one.
  bool contains(const BoundingBox &bbox) const {
    return contains(bbox.minPoint) && contains(bbox.maxPoint);
  }

  /// Checks whether given coordinate inside the bounding box.
  bool contains(const GeoCoordinate &coordinate) const {
    return coordinate.latitude > minPoint.latitude && coordinate.longitude > minPoint.longitude &&
        coordinate.latitude < maxPoint.latitude && coordinate.longitude < maxPoint.longitude;
  }

  /// Checks whether given bounding box intersects the current one.
  bool intersects(const BoundingBox &rhs) const {
    double minLat = std::max(rhs.minPoint.latitude, minPoint.latitude);
    double minLon = std::max(rhs.minPoint.longitude, minPoint.longitude);
    double maxLat = std::min(rhs.maxPoint.latitude, maxPoint.latitude);
    double maxLon = std::min(rhs.maxPoint.longitude, maxPoint.longitude);

    return minLat <= maxLat && minLon <= maxLon;
  }

  /// Returns center of bounding box.
  GeoCoordinate center() const {
    return GeoCoordinate(
        minPoint.latitude + (maxPoint.latitude - minPoint.latitude)/2,
        minPoint.longitude + (maxPoint.longitude - minPoint.longitude)/2);
  }

  /// Returns width.
  double width() const {
    return maxPoint.longitude - minPoint.longitude;
  }

  /// Returns height.
  double height() const {
    return maxPoint.latitude - minPoint.latitude;
  }
};

}
#endif // BOUNDINGBOX_HPP_DEFINED
