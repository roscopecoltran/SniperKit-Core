#ifndef BUILDERS_BUILDINGS_FACADES_DOMEROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_FACADES_DOMEROOFBUILDER_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "builders/buildings/roofs/RoofBuilder.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"
#include "utils/GeometryUtils.hpp"
#include "utils/GeoUtils.hpp"

namespace utymap {
namespace builders {

/// Builds flat roof in low poly.
class DomeRoofBuilder : public RoofBuilder {
 public:
  DomeRoofBuilder(const utymap::builders::BuilderContext &builderContext,
                  utymap::builders::MeshContext &meshContext) :
      RoofBuilder(builderContext, meshContext) {
  }

  void build(utymap::math::Polygon &polygon) override {
    utymap::utils::outerRectangles(polygon, [&](const utymap::math::Rectangle &rectangle) {
      utymap::math::Vector2 center2d;
      utymap::math::Vector2 size;
      utymap::utils::getCircle(rectangle, center2d, size);

      double heightInMeters = utymap::utils::GeoUtils::distance(
          utymap::GeoCoordinate(center2d.y, center2d.x),
          utymap::GeoCoordinate(center2d.y + size.y, center2d.x));

      utymap::builders::IcoSphereGenerator generator(builderContext_, meshContext_);
      generator
          .setCenter(utymap::math::Vector3(center2d.x, minHeight_, center2d.y))
          .setSize(utymap::math::Vector3(size.x, heightInMeters, size.y))
          .setRecursionLevel(2)
          .isSemiSphere(true)
          .setVertexNoiseFreq(0.0)
          .setColorNoiseFreq(0)
          .generate();

      builderContext_.meshBuilder.writeTextureMappingInfo(meshContext_.mesh, meshContext_.appearanceOptions);
    });
  }
};

}
}
#endif // BUILDERS_BUILDINGS_FACADES_DOMEROOFBUILDER_HPP_DEFINED
