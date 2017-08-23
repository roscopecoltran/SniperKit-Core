#ifndef UTILS_MESHUTILS_HPP_DEFINED
#define UTILS_MESHUTILS_HPP_DEFINED

#include "math/Mesh.hpp"
#include "math/Vector3.hpp"

namespace utymap {
namespace utils {

/// Copies mesh into existing one with offset.
inline void copyMesh(const utymap::math::Vector3 &offset,
                     const utymap::math::Mesh &source,
                     utymap::math::Mesh &destination) {
  int startIndex = static_cast<int>(destination.vertices.size()/3);

  // copy adjusted vertices
  for (std::size_t i = 0; i < source.vertices.size();) {
    destination.vertices.push_back(source.vertices[i++] + offset.x);
    destination.vertices.push_back(source.vertices[i++] + offset.z);
    destination.vertices.push_back(source.vertices[i++] + offset.y);
  }

  // copy adjusted triangles
  std::transform(source.triangles.begin(),
                 source.triangles.end(),
                 std::back_inserter(destination.triangles),
                 [&](int value) {
                   return value + startIndex;
                 });

  // copy colors
  std::copy(source.colors.begin(), source.colors.end(), std::back_inserter(destination.colors));

  // copy adjusted uvs
  std::copy(source.uvs.begin(), source.uvs.end(), std::back_inserter(destination.uvs));
  std::copy(source.uvMap.begin(), source.uvMap.end(), std::back_inserter(destination.uvMap));

  startIndex = static_cast<int>(destination.uvs.size() - source.uvs.size());
  for (std::size_t i = destination.uvMap.size() - source.uvMap.size(); i < destination.uvMap.size(); i += 8) {
    destination.uvMap[i] += startIndex;
  }
}

/// Copies mesh along two coordinates.
inline void copyMeshAlong(const utymap::QuadKey &quadKey, const utymap::GeoCoordinate &position,
                          const utymap::GeoCoordinate &p1, const utymap::GeoCoordinate &p2,
                          const utymap::math::Mesh &source, utymap::math::Mesh &destination, double stepInMeters,
                          const utymap::heightmap::ElevationProvider &eleProvider) {
  double distanceInMeters = GeoUtils::distance(p1, p2);
  int count = static_cast<int>(distanceInMeters/stepInMeters);

  for (int j = 0; j < count; ++j) {
    GeoCoordinate newPosition = GeoUtils::newPoint(p1, p2, static_cast<double>(j)/count);

    const auto elevation = eleProvider.getElevation(quadKey, newPosition);
    const auto offset = utymap::math::Vector3(newPosition.longitude - position.longitude,
                                              elevation,
                                              newPosition.latitude - position.latitude);
    utymap::utils::copyMesh(offset, source, destination);
  }
}

}
}

#endif // UTILS_GEOUTILS_HPP_DEFINED
