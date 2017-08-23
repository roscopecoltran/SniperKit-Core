#ifndef UTILS_GEOUTILS_HPP_DEFINED
#define UTILS_GEOUTILS_HPP_DEFINED

#include "BoundingBox.hpp"
#include "QuadKey.hpp"
#include "utils/MathUtils.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace utymap {
namespace utils {

class GeoUtils final {
  /// The circumference at the equator (latitude 0)
  static const int LatitudeEquator = 40075160;
  /// Distance of full circle around the earth through the poles.
  static const int CircleDistance = 40008000;

 public:
  static const int MinLevelOfDetails = 1;
  static const int MaxLevelOfDetails = 19;

  /// Converts Latitude/Longitude to quadkey
  static QuadKey GeoCoordinateToQuadKey(const GeoCoordinate &coordinate, int levelOfDetail) {
    return QuadKey(levelOfDetail,
                   lonToTileX(clamp(coordinate.longitude, -179.9999999, 179.9999999), levelOfDetail),
                   latToTileY(clamp(coordinate.latitude, -85.05112877, 85.05112877), levelOfDetail));
  }

  /// Converts given world coordinate represented by (x, y) to geocoordinate.
  static GeoCoordinate worldToGeo(const GeoCoordinate &worldZero, double x, double y) {
    double latitudeCircumference = LatitudeEquator*std::cos(utymap::utils::deg2Rad(worldZero.latitude));

    auto deltaLongitude = (x*360)/latitudeCircumference;
    auto deltaLatitude = (y*360)/CircleDistance;

    return GeoCoordinate(worldZero.latitude + deltaLatitude, worldZero.longitude + deltaLongitude);
  }

  /// Converts quadkey to bounding box
  static BoundingBox quadKeyToBoundingBox(const QuadKey &quadKey) {
    GeoCoordinate minPoint, maxPoint;
    int levelOfDetail = quadKey.levelOfDetail;
    minPoint.latitude = tileYToLat(quadKey.tileY + 1, levelOfDetail);
    maxPoint.latitude = tileYToLat(quadKey.tileY, levelOfDetail);
    minPoint.longitude = tileXToLon(quadKey.tileX, levelOfDetail);
    maxPoint.longitude = tileXToLon(quadKey.tileX + 1, levelOfDetail);

    return BoundingBox(minPoint, maxPoint);
  }

  static std::string quadKeyToString(const QuadKey &quadKey) {
    std::string code;
    code.reserve(static_cast<std::size_t>(quadKey.levelOfDetail));
    for (int i = quadKey.levelOfDetail; i > 0; --i) {
      char digit = '0';
      int mask = 1 << (i - 1);
      if ((quadKey.tileX & mask)!=0)
        digit++;
      if ((quadKey.tileY & mask)!=0) {
        digit++;
        digit++;
      }
      code += digit;
    }
    return code;
  }

  /// Visits all tiles which are intersecting with given bounding box at given level of details
  template<typename Visitor>
  static void visitTileRange(const BoundingBox &bbox, int levelOfDetail, const Visitor &visitor) {
    if (!bbox.isValid()) return;

    QuadKey start = GeoCoordinateToQuadKey(bbox.minPoint, levelOfDetail);
    QuadKey end = GeoCoordinateToQuadKey(bbox.maxPoint, levelOfDetail);

    for (int y = end.tileY; y < start.tileY + 1; y++) {
      for (int x = start.tileX; x < end.tileX + 1; x++) {
        const QuadKey currentQuadKey = {levelOfDetail, x, y};
        const BoundingBox currentBbox = quadKeyToBoundingBox(currentQuadKey);
        if (bbox.intersects(currentBbox)) {
          visitor(currentQuadKey, currentBbox);
        }
      }
    }
  }

  /// Checks whether given point inside polygon.
  template<typename Iter>
  static bool isPointInPolygon(const GeoCoordinate &point, Iter begin, Iter end) {
    bool c = false;
    for (auto iCoord = begin, jCoord = end - 1; iCoord!=end; jCoord = iCoord++) {
      if (((iCoord->latitude > point.latitude)!=(jCoord->latitude > point.latitude)) &&
          (point.longitude < (jCoord->longitude - iCoord->longitude)*(point.latitude - iCoord->latitude)/
              (jCoord->latitude - iCoord->latitude) + iCoord->longitude))
        c = !c;
    }
    return c;
  }

  /// Gets offset in degrees
  static double getOffset(const GeoCoordinate &point, double offsetInMeters) {
    double lat = deg2Rad(point.latitude);
    // Radius of Earth at given latitude
    double radius = wgs84EarthRadius(lat);
    return rad2Deg(offsetInMeters/radius);
  }

  /// Gets scaled value.
  static double getScaled(const BoundingBox &bbox, double scale, double value) {
    return getOffset(bbox.center(), value)*scale/bbox.width();
  }

  /// Gets distance in meters for given coordinate
  static double distance(const GeoCoordinate &p1, const GeoCoordinate &p2) {
    double dLat = deg2Rad(p1.latitude - p2.latitude);
    double dLon = deg2Rad(p1.longitude - p2.longitude);

    double lat1 = deg2Rad(p1.latitude);
    double lat2 = deg2Rad(p2.latitude);

    double a = std::sin(dLat/2)*std::sin(dLat/2) +
        std::sin(dLon/2)*std::sin(dLon/2)*std::cos(lat1)*std::cos(lat2);

    double c = 2*std::atan2(std::sqrt(a), std::sqrt(1 - a));

    double radius = wgs84EarthRadius(dLat);

    return radius*c;
  }

  /// Returns point between two at given distance in percents.
  static GeoCoordinate newPoint(const GeoCoordinate &p1,
                                const GeoCoordinate &p2,
                                double distanceInProcents) {
    return GeoCoordinate(p1.latitude + (p2.latitude - p1.latitude)*distanceInProcents,
                         p1.longitude + (p2.longitude - p1.longitude)*distanceInProcents);
  }

 private:

  static int lonToTileX(double lon, int levelOfDetail) {
    return static_cast<int>(std::floor((lon + 180.0)/360.0*pow(2.0, levelOfDetail)));
  }

  static int latToTileY(double lat, int levelOfDetail) {
    return static_cast<int>(std::floor(
        (1.0 - log(tan(lat*pi/180.0) + 1.0/cos(lat*pi/180.0))/pi)/2.0*pow(2.0, levelOfDetail)));
  }

  static double tileXToLon(int x, int levelOfDetail) {
    return x/pow(2.0, levelOfDetail)*360.0 - 180;
  }

  static double tileYToLat(int y, int levelOfDetail) {
    double n = pi - 2.0*pi*y/pow(2.0, levelOfDetail);
    return 180.0/pi*atan(0.5*(exp(n) - exp(-n)));
  }

  /// Earth radius at a given latitude, according to the WGS-84 ellipsoid [m].
  static double wgs84EarthRadius(double lat) {
    // Semi-axes of WGS-84 geoidal reference
    const double WGS84_a = 6378137.0; // Major semiaxis [m]
    const double WGS84_b = 6356752.3; // Minor semiaxis [m]

    // http://en.wikipedia.org/wiki/Earth_radius
    auto an = WGS84_a*WGS84_a*std::cos(lat);
    auto bn = WGS84_b*WGS84_b*std::sin(lat);
    auto ad = WGS84_a*std::cos(lat);
    auto bd = WGS84_b*std::sin(lat);
    return std::sqrt((an*an + bn*bn)/(ad*ad + bd*bd));
  }
};

}
}

#endif // UTILS_GEOUTILS_HPP_DEFINED
