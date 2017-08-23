#ifndef BUILDERS_BUILDINGS_ROOFS_ROUNDROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_ROOFS_ROUNDROOFBUILDER_HPP_DEFINED

#include "builders/buildings/roofs/FlatRoofBuilder.hpp"
#include "builders/MeshBuilder.hpp"
#include "math/Quaternion.hpp"
#include "utils/MathUtils.hpp"

namespace utymap {
namespace builders {

/// Builds round roof. So far, supports only simple rectangle roofs.
class RoundRoofBuilder : public FlatRoofBuilder {
  /// Specifies ridge direction (orientation).
  enum class Direction { Along, Across };
  /// Specifies amount of cirecle segments.
  const int RadialSegmentCount = 7;

 public:
  RoundRoofBuilder(const utymap::builders::BuilderContext &builderContext,
                   utymap::builders::MeshContext &meshContext) :
      FlatRoofBuilder(builderContext, meshContext), direction_() {
  }

  /// Sets roof direction. It should either be a string orientation (N, NNE, etc.)
  /// or an angle in degree from north clockwise
  void setDirection(const std::string &direction) override {
    direction_ = direction=="across" ? Direction::Across : Direction::Along;
  }

  void build(utymap::math::Polygon &polygon) override {
    if (!buildRound(polygon)) {
      FlatRoofBuilder::build(polygon);
      return;
    }

    builderContext_.meshBuilder
        .writeTextureMappingInfo(meshContext_.mesh, meshContext_.appearanceOptions);
  }

 private:

  /// Tries to build round roof, returns false if polygon has unexpected shape.
  bool buildRound(utymap::math::Polygon &polygon) const {
    if (!polygon.inners.empty())
      return false;

    for (const auto range : polygon.outers) {
      // detect side with maximum length
      double maxSideLength = 0;
      auto maxSideIndex = range.first;
      const auto lastPointIndex = range.second - 2;
      for (std::size_t i = range.first; i < range.second; i += 2) {
        const auto nextIndex = i==lastPointIndex ? range.first : i + 2;

        utymap::math::Vector2 v0(polygon.points[i], polygon.points[i + 1]);
        utymap::math::Vector2 v1(polygon.points[nextIndex], polygon.points[nextIndex + 1]);

        auto length = utymap::math::Vector2::distance(v0, v1);
        if (length > maxSideLength) {
          maxSideIndex = i;
          maxSideLength = length;
        }
      }

      // specify points for sides and front/back
      const utymap::math::Vector3 p0(polygon.points[maxSideIndex], minHeight_, polygon.points[maxSideIndex + 1]);
      auto next = nextIndex(maxSideIndex, range.first, lastPointIndex);
      const utymap::math::Vector3 p1(polygon.points[next], minHeight_, polygon.points[next + 1]);
      next = nextIndex(next, range.first, lastPointIndex);
      const utymap::math::Vector3 p2(polygon.points[next], minHeight_, polygon.points[next + 1]);
      next = nextIndex(next, range.first, lastPointIndex);
      const utymap::math::Vector3 p3(polygon.points[next], minHeight_, polygon.points[next + 1]);

      // build round shape based on orientation
      if (direction_==Direction::Along)
        buildRoundShape(p0, p1, p2, p3);
      else
        buildRoundShape(p1, p2, p3, p0);
    }

    return true;
  }

  /// Builds round shape around front and back.  p1-p2 and p0-p3 are radial segments
  void buildRoundShape(const utymap::math::Vector3 &p0, const utymap::math::Vector3 &p1,
                       const utymap::math::Vector3 &p2, const utymap::math::Vector3 &p3) const {
    // define uv mapping
    utymap::math::Vector2 u0(0, 0);

    // prepare data involved into rotation logic.
    const auto direction2d = (p3 - p0).normalized();
    const utymap::math::Vector3 direction(-direction2d.z, 0, direction2d.x);
    // these points are rotation centers for front/back sides.
    const auto frontCenter = (p0 + p3)/2;
    const auto backCenter = (p1 + p2)/2;
    // these vectors will be rotated.
    const auto frontRot = (p0 - frontCenter).normalized();
    const auto backRot = (p1 - backCenter).normalized();
    // these radius are used to restore vector magnitudes.
    const double frontRadius = utymap::math::Vector3::distance(p0, frontCenter);
    const double backRadius = utymap::math::Vector3::distance(p1, backCenter);
    // quaternions between which spherical interpolation should be done.
    const auto q0 = utymap::math::Quaternion::fromAngleAxis(0, direction);
    const auto q1 = utymap::math::Quaternion::fromAngleAxis(-pi, direction);

    for (int j = 0; j < RadialSegmentCount; j++) {
      auto q0t = q0.slerp(q0, q1, j/static_cast<double>(RadialSegmentCount));
      auto q1t = q1.slerp(q0, q1, (j + 1)/static_cast<double>(RadialSegmentCount));

      const auto front1 = restore(q0t*frontRot, frontRadius, frontCenter);
      const auto front2 = restore(q1t*frontRot, frontRadius, frontCenter);
      const auto back1 = restore(q0t*backRot, backRadius, backCenter);
      const auto back2 = restore(q1t*backRot, backRadius, backCenter);

      addTriangle(frontCenter, front2, front1, u0, u0, u0);
      addTriangle(backCenter, back1, back2, u0, u0, u0);

      addTriangle(front1, back2, back1, u0, u0, u0);
      addTriangle(front2, back2, front1, u0, u0, u0);
    }
  }

  std::size_t nextIndex(std::size_t current, std::size_t min, std::size_t max) const {
    return (current += 2) > max ? min : current;
  }

  utymap::math::Vector3 restore(const utymap::math::Vector3 &v,
                                double radius,
                                const utymap::math::Vector3 &center) const {
    // p' = q * p * qInversed
    return utymap::math::Vector3(center.x + v.x*radius, minHeight_ + v.y*height_, center.z + v.z*radius);
  }

  Direction direction_;
};

}
}

#endif // BUILDERS_BUILDINGS_ROOFS_ROUNDROOFBUILDER_HPP_DEFINED
