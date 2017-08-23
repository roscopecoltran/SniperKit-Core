#ifndef BUILDERS_BUILDINGS_FACADES_SPHEREFACADEBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_FACADES_SPHEREFACADEBUILDER_HPP_DEFINED

#include "builders/buildings/facades/FacadeBuilder.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"
#include "math/Polygon.hpp"
#include "math/Vector2.hpp"
#include "utils/GeometryUtils.hpp"

namespace utymap {
namespace builders {

/// Responsible for building cylinder shaped facade.
class SphereFacadeBuilder : public FacadeBuilder {
 public:
  SphereFacadeBuilder(const utymap::builders::BuilderContext &builderContext,
                      utymap::builders::MeshContext &meshContext) :
      FacadeBuilder(builderContext, meshContext) {
  }

  void build(utymap::math::Polygon &polygon) override {
    utymap::math::Vector2 center2d;
    utymap::math::Vector2 size;
    utymap::utils::getCircle(polygon.rectangle, center2d, size);

    double heightInMeters = utymap::utils::GeoUtils::distance(
        utymap::GeoCoordinate(center2d.y, center2d.x),
        utymap::GeoCoordinate(center2d.y + size.y, center2d.x));

    IcoSphereGenerator icoSphereGenerator(builderContext_, meshContext_);
    icoSphereGenerator
        .setCenter(utymap::math::Vector3(center2d.x, minHeight_, center2d.y))
        .setSize(utymap::math::Vector3(size.x, heightInMeters, size.y))
        .setRecursionLevel(2)
        .isSemiSphere(false)
        .setColorNoiseFreq(0)
        .generate();

    builderContext_.meshBuilder.writeTextureMappingInfo(meshContext_.mesh, meshContext_.appearanceOptions);
  }
};

}
}
#endif // BUILDERS_BUILDINGS_FACADES_SPHEREFACADEBUILDER_HPP_DEFINED
