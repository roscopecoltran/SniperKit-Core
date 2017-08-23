#ifndef UTILS_GEOMETRYUTILS_HPP_DEFINED
#define UTILS_GEOMETRYUTILS_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "mapcss/Style.hpp"
#include "math/Polygon.hpp"
#include "math/Rectangle.hpp"
#include "math/Vector2.hpp"
#include "math/Vector3.hpp"

#include <algorithm>

namespace utymap {
namespace utils {

/// Gets area of polygon
template<typename T>
double getArea(const T &coordinates) {
  auto size = coordinates.size();
  double area = 0.0;
  for (std::size_t p = size - 1, q = 0; q < size; p = q++)
    area += coordinates[p].longitude*coordinates[q].latitude - coordinates[q].longitude*coordinates[p].latitude;
  return area;
}

/// Checks whether geocoordinates are in clockwise oreder
template<typename T>
bool isClockwise(const T &coordinates) {
  return getArea(coordinates) < 0;
}

/// Gets circle parameters from geocoordinates
inline void getCircle(const std::vector<utymap::GeoCoordinate> &coordinates,
                      utymap::GeoCoordinate &center,
                      double &radius) {
  auto minMaxLong = std::minmax_element(
      coordinates.begin(),
      coordinates.end(),
      [](const utymap::GeoCoordinate &left, const utymap::GeoCoordinate &right) {
        return left.longitude < right.longitude;
      });

  auto minMaxLat = std::minmax_element(
      coordinates.begin(),
      coordinates.end(),
      [](const utymap::GeoCoordinate &left, const utymap::GeoCoordinate &right) {
        return left.latitude < right.latitude;
      });

  double centerLon = (minMaxLong.first->longitude + (minMaxLong.second->longitude - minMaxLong.first->longitude)/2);
  double centerLat = (minMaxLat.first->latitude + (minMaxLat.second->latitude - minMaxLat.first->latitude)/2);

  radius = (minMaxLat.second->latitude - minMaxLat.first->latitude)/2;
  center = utymap::GeoCoordinate(centerLat, centerLon);
}

/// Gets circle parameters from rectangle.
inline void getCircle(const utymap::math::Rectangle &rectangle,
                      utymap::math::Vector2 &center,
                      utymap::math::Vector2 &size) {
  double radiusX = (rectangle.xMax - rectangle.xMin)/2;
  double radiusY = (rectangle.yMax - rectangle.yMin)/2;

  double centerX = rectangle.xMin + radiusX;
  double centerY = rectangle.yMin + radiusY;

  size = utymap::math::Vector2(radiusX, radiusY);
  center = utymap::math::Vector2(centerX, centerY);
}

/// Gets centroid.
inline utymap::math::Vector2 getCentroid(const utymap::math::Polygon &polygon,
                                         const utymap::math::Polygon::Range &range) {
  double centroidX = 0.0;
  double centroidY = 0.0;

  for (std::size_t i = range.first; i < range.second; i += 2) {
    centroidX += polygon.points[i];
    centroidY += polygon.points[i + 1];
  }

  auto count = static_cast<double>(range.second - range.first)/2.;
  centroidX /= count;
  centroidY /= count;

  return utymap::math::Vector2(centroidX, centroidY);
}

/// Gets size as Vector3 with scaled axis values.
inline utymap::math::Vector3 getSize(const utymap::BoundingBox boundingBox,
                                     const utymap::mapcss::Style &style,
                                     const std::string &key) {
  double radiusInDegrees = style.getValue(key, boundingBox);
  double radiusInMeters = style.getValue(key, boundingBox.height());
  return utymap::math::Vector3(1.5*radiusInDegrees, radiusInMeters, radiusInDegrees);
}

/// Iterates through polygon outers and call visitor with rectangle of this outer
template<typename Visitor>
void outerRectangles(const utymap::math::Polygon &polygon, const Visitor &visitor) {
  if (polygon.outers.size()==1) {
    visitor(polygon.rectangle);
    return;
  }

  for (const auto &outer : polygon.outers) {
    auto rectangle = utymap::math::Rectangle();
    for (auto i = outer.first; i < outer.second; i += 2) {
      rectangle.expand(utymap::math::Vector2(polygon.points[i], polygon.points[i + 1]));
    }
    visitor(rectangle);
  }
}

/// Gets intersection point between two segments. If they don't intersect double lowest is returned.
inline double getIntersection(const utymap::math::Vector2 &start1,
                              const utymap::math::Vector2 &end1,
                              const utymap::math::Vector2 &start2,
                              const utymap::math::Vector2 &end2) {
  double denom = ((end1.x - start1.x)*(end2.y - start2.y)) - ((end1.y - start1.y)*(end2.x - start2.x));

  //  AB & CD are parallel
  if (denom==0)
    return std::numeric_limits<double>::lowest();

  double numer = ((start1.y - start2.y)*(end2.x - start2.x)) - ((start1.x - start2.x)*(end2.y - start2.y));

  double r = numer/denom;

  double numer2 = ((start1.y - start2.y)*(end1.x - start1.x)) - ((start1.x - start2.x)*(end1.y - start1.y));

  double s = numer2/denom;

  return (r < 0 || r > 1) || (s < 0 || s > 1)
         ? std::numeric_limits<double>::lowest()
         : r;
}

/// Gets point along the line. r is computed by getIntersection
inline utymap::math::Vector2 getPointAlongLine(const utymap::math::Vector2 &start,
                                               const utymap::math::Vector2 &end,
                                               double r) {
  return utymap::math::Vector2(start.x + (r*(end.x - start.x)), start.y + (r*(end.y - start.y)));
}

/// Gets list of vertices received by segment offsetting
inline std::vector<utymap::math::Vector2> getOffsetLine(const utymap::math::Vector2 &start,
                                                        const utymap::math::Vector2 &end,
                                                        double width) {
  auto direction = (end - start).normalized();
  utymap::math::Vector2 n(-direction.y, direction.x);

  std::vector<utymap::math::Vector2> vertices;
  vertices.reserve(4);

  vertices.push_back(start + n*width);
  vertices.push_back(start - n*width);
  vertices.push_back(end - n*width);
  vertices.push_back(end + n*width);

  return vertices;
}

/// Returns true if given point is inside given simple polygon (including borders) represented by points.
inline bool isPointInPolygon(const utymap::math::Vector2 &p,
                             const std::vector<utymap::math::Vector2> &points) {
  std::size_t i, j, size = points.size();
  bool result = false;

  for (i = 0, j = size - 1; i < size; j = i++) {
    if (((points[i].y >= p.y)!=(points[j].y >= p.y)) &&
        (p.x <= (points[j].x - points[i].x)*(p.y - points[i].y)/(points[j].y - points[i].y) + points[i].x))
      result = !result;
  }

  return result;
}

}
}

#endif // UTILS_GEOMETRYUTILS_HPP_DEFINED
