#ifndef BUILDERS_BUILDINGS_FACADES_CYLINDERFACADEBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_FACADES_CYLINDERFACADEBUILDER_HPP_DEFINED

#include "builders/buildings/facades/FacadeBuilder.hpp"
#include "builders/generators/CylinderGenerator.hpp"
#include "utils/GeometryUtils.hpp"

namespace utymap {
namespace builders {

/// Responsible for building cylinder shaped facade.
class CylinderFacadeBuilder : public FacadeBuilder {
 public:
  CylinderFacadeBuilder(const utymap::builders::BuilderContext &builderContext,
                        utymap::builders::MeshContext &meshContext) :
      FacadeBuilder(builderContext, meshContext) {
  }

  void build(utymap::math::Polygon &polygon) override {
    utymap::utils::outerRectangles(polygon, [&](const utymap::math::Rectangle &rectangle) {
      utymap::math::Vector2 center2d;
      utymap::math::Vector2 size;
      utymap::utils::getCircle(rectangle, center2d, size);

      CylinderGenerator cylinderGenerator(builderContext_, meshContext_);
      cylinderGenerator
          .setCenter(utymap::math::Vector3(center2d.x, minHeight_, center2d.y))
          .setMaxSegmentHeight(5)
          .setRadialSegments(7)
          .setSize(utymap::math::Vector3(size.x, height_, size.y))
          .setColorNoiseFreq(0)
          .generate();

      builderContext_.meshBuilder.writeTextureMappingInfo(meshContext_.mesh, meshContext_.appearanceOptions);
    });
  }
};

}
}
#endif // BUILDERS_BUILDINGS_FACADES_CYLINDERFACADEBUILDER_HPP_DEFINED
