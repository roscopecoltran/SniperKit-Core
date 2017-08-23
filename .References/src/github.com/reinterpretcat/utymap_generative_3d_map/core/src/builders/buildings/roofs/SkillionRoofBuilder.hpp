#ifndef BUILDERS_BUILDINGS_ROOFS_SKILLIONROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_ROOFS_SKILLIONROOFBUILDER_HPP_DEFINED

#include "builders/buildings/roofs/FlatRoofBuilder.hpp"
#include "builders/MeshBuilder.hpp"
#include "utils/CoreUtils.hpp"
#include "utils/GeometryUtils.hpp"
#include "utils/MathUtils.hpp"
#include "utils/MeshUtils.hpp"

namespace utymap {
namespace builders {

/// Builds skillion roof. So far, supports only simple rectangle roofs.
class SkillionRoofBuilder : public FlatRoofBuilder {
 public:
  SkillionRoofBuilder(const utymap::builders::BuilderContext &builderContext,
                      utymap::builders::MeshContext &meshContext) :
      FlatRoofBuilder(builderContext, meshContext), direction_(0) {
  }

  /// Sets roof direction. It should either be a string orientation (N, NNE, etc.)
  /// or an angle in degree from north clockwise
  void setDirection(const std::string &direction) override {
    // see http://wiki.openstreetmap.org/wiki/Key:roof:direction
    static std::unordered_map<std::string, double> directions_ = {
        {"N", 0}, {"NNE", 22}, {"NE", 45}, {"ENE", 67},
        {"E", 90}, {"ESE", 112}, {"SE", 135}, {"SSE", 157},
        {"S", 180}, {"SSW", 202}, {"SW", 225}, {"WSW", 247},
        {"W", 270}, {"WNW", 292}, {"NW", 315}, {"NNW", 337}};
    auto dir = directions_.find(direction);
    direction_ = dir!=directions_.end()
                 ? dir->second
                 : utymap::utils::parseDouble(direction);
  }

  void build(utymap::math::Polygon &polygon) override {
    if (!buildSkillion(polygon)) {
      FlatRoofBuilder::build(polygon);
      return;
    }

    builderContext_.meshBuilder
        .writeTextureMappingInfo(meshContext_.mesh, meshContext_.appearanceOptions);
  }

 private:

  /// Tries to build skillion roof. So far we support only one simple polygon.
  bool buildSkillion(utymap::math::Polygon &polygon) const {
    // get direction vector
    const auto grad = utymap::utils::deg2Rad(direction_);
    const auto direction = utymap::math::Vector2(std::sin(grad), std::cos(grad)).normalized();
    const auto maxHeight = minHeight_ + height_;

    for (const auto &range : polygon.outers) {
      // get center and points outside front/back from center in specified direction
      const auto center = utymap::utils::getCentroid(polygon, range);
      const auto outBackPoint = center - direction*0.1;
      const auto outFrontPoint = center + direction*0.1;

      // copy geometry options and change some values to control mesh builder behaviour
      auto geometryOptions = meshContext_.geometryOptions;
      geometryOptions.heightOffset = minHeight_;
      geometryOptions.elevation = 0;
      geometryOptions.area = 0;

      // build mesh mostly to have triangulation in place
      utymap::math::Mesh mesh("");
      builderContext_.meshBuilder.addPolygon(mesh, polygon, geometryOptions, meshContext_.appearanceOptions);

      // detect front/back sides to set min/max elevation and get roof plane equation
      std::size_t frontSideIndex = mesh.vertices.size();
      std::size_t topBackSideIndex = mesh.vertices.size();
      double minDistance = std::numeric_limits<double>::max();
      double maxDistance = 0;
      const auto lastPointIndex = mesh.vertices.size() - 3;
      for (std::size_t i = 0; i < mesh.vertices.size(); i += 3) {
        auto nextIndex = i==lastPointIndex ? 0 : i + 3;

        utymap::math::Vector2 v0(mesh.vertices[i], mesh.vertices[i + 1]);
        utymap::math::Vector2 v1(mesh.vertices[nextIndex], mesh.vertices[nextIndex + 1]);

        double r = utymap::utils::getIntersection(v0, v1, outBackPoint, outFrontPoint);
        if (r > std::numeric_limits<double>::lowest()) {
          const auto intersection = utymap::utils::getPointAlongLine(v0, v1, r);
          auto distance = utymap::math::Vector2::distance(outBackPoint, intersection);

          if (distance > maxDistance) { // Found new front face
            frontSideIndex = i;
            maxDistance = distance;
          }
          if (distance < minDistance) { // Found new the highest point on back side
            topBackSideIndex = (utymap::math::Vector2::distance(v0, intersection) <
                utymap::math::Vector2::distance(v1, intersection)) ? i : nextIndex;
            minDistance = distance;
          }
        }
      }

      // fail to determine front/back: fallback to flat roof
      if (frontSideIndex > lastPointIndex || topBackSideIndex > lastPointIndex || frontSideIndex==topBackSideIndex)
        return false;

      // define points which are on top roof plane
      utymap::math::Vector3 p1(mesh.vertices[frontSideIndex], minHeight_, mesh.vertices[frontSideIndex + 1]);
      auto nextFrontSideIndex = frontSideIndex==lastPointIndex ? 0 : frontSideIndex + 3;
      utymap::math::Vector3 p2(mesh.vertices[nextFrontSideIndex], minHeight_, mesh.vertices[nextFrontSideIndex + 1]);
      utymap::math::Vector3 p3(mesh.vertices[topBackSideIndex], maxHeight, mesh.vertices[topBackSideIndex + 1]);

      // calculate equation of plane in classical form: Ax + By + Cz = d where n is (A, B, C)
      auto n = utymap::math::Vector3::cross(p1 - p2, p3 - p2);
      double d = n.x*p1.x + n.y*p1.y + n.z*p1.z;

      // loop over all vertices, calculate their height
      for (std::size_t i = 0; i < mesh.vertices.size(); i += 3) {
        if (i==frontSideIndex || i==nextFrontSideIndex)
          continue;
        utymap::math::Vector2 p(mesh.vertices[i], mesh.vertices[i + 1]);
        mesh.vertices[i + 2] = utymap::utils::clamp(calcHeight(p, n, d), minHeight_, maxHeight);
      }

      // build faces
      double scale = utymap::utils::GeoUtils::getScaled(builderContext_.boundingBox,
                                                        meshContext_.appearanceOptions.textureScale, height_);
      utymap::math::Vector2 u0(0, 0);
      utymap::math::Vector2 u1(0, scale);
      utymap::math::Vector2 u2(scale, scale);
      utymap::math::Vector2 u3(scale, 0);

      for (std::size_t i = 0; i < mesh.vertices.size(); i += 3) {
        if (i==frontSideIndex)
          continue;

        auto nextIndex = i==lastPointIndex ? 0 : i + 3;

        utymap::math::Vector3 v0(mesh.vertices[i], minHeight_, mesh.vertices[i + 1]);
        utymap::math::Vector3 v1(mesh.vertices[i], mesh.vertices[i + 2], mesh.vertices[i + 1]);
        utymap::math::Vector3 v2(mesh.vertices[nextIndex], mesh.vertices[nextIndex + 2], mesh.vertices[nextIndex + 1]);
        utymap::math::Vector3 v3(mesh.vertices[nextIndex], minHeight_, mesh.vertices[nextIndex + 1]);

        if (i==nextFrontSideIndex)
          addTriangle(v0, v2, v3, u0, u2, u3);
        else if (nextIndex==frontSideIndex)
          addTriangle(v0, v1, v3, u0, u1, u3);
        else {
          addTriangle(v0, v2, v3, u0, u2, u3);
          addTriangle(v0, v1, v2, u0, u1, u2);
        }
      }

      utymap::utils::copyMesh(utymap::math::Vector3(0, 0, 0), mesh, meshContext_.mesh);
    }

    return true;
  }

  /// Calculates height of the 2d point using plane equation.
  static double calcHeight(const utymap::math::Vector2 &p, const utymap::math::Vector3 &n, double d) {
    return n.y!=0 ? (d - n.x*p.x - n.z*p.y)/n.y : 0;
  }

  double direction_;
};

}
}

#endif // BUILDERS_BUILDINGS_ROOFS_SKILLIONROOFBUILDER_HPP_DEFINED
