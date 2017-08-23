#ifndef BUILDERS_BUILDINGS_ROOFS_FLATROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_ROOFS_FLATROOFBUILDER_HPP_DEFINED

#include "builders/buildings/roofs/RoofBuilder.hpp"
#include "builders/MeshBuilder.hpp"

namespace utymap {
namespace builders {

/// Builds flat roof in low poly.
class FlatRoofBuilder : public RoofBuilder {
 public:
  FlatRoofBuilder(const utymap::builders::BuilderContext &builderContext,
                  utymap::builders::MeshContext &meshContext) :
      RoofBuilder(builderContext, meshContext) {
  }

  FlatRoofBuilder &flipSide() {
    meshContext_.geometryOptions.flipSide = true;
    return *this;
  }

  void build(utymap::math::Polygon &polygon) override {
    meshContext_.geometryOptions.elevation = minHeight_;
    meshContext_.geometryOptions.heightOffset = 0;

    builderContext_.meshBuilder
        .addPolygon(meshContext_.mesh,
                    polygon,
                    meshContext_.geometryOptions,
                    meshContext_.appearanceOptions);
  }
};

}
}
#endif // BUILDERS_BUILDINGS_ROOFS_FLATROOFBUILDER_HPP_DEFINED
