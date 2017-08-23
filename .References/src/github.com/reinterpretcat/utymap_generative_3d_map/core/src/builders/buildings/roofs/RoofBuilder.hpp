#ifndef BUILDERS_BUILDINGS_ROOFS_ROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_ROOFS_ROOFBUILDER_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/MeshContext.hpp"
#include "math/Polygon.hpp"

namespace utymap {
namespace builders {

/// Specifies base roof builder functionality.
class RoofBuilder {
 public:
  RoofBuilder(const utymap::builders::BuilderContext &builderContext,
              utymap::builders::MeshContext &meshContext) :
      builderContext_(builderContext),
      meshContext_(meshContext),
      height_(0),
      minHeight_(0) {
  }

  virtual ~RoofBuilder() = default;

  // Sets height.
  RoofBuilder &setHeight(double height) {
    height_ = height;
    return *this;
  }

  // Sets height above ground level.
  RoofBuilder &setMinHeight(double minHeight) {
    minHeight_ = minHeight;
    return *this;
  }

  // Sets color noise freq.
  RoofBuilder &setColorNoiseFreq(double noiseFreq) {
    meshContext_.appearanceOptions.colorNoiseFreq = noiseFreq;
    return *this;
  }

  // Sets direction of roof.
  virtual void setDirection(const std::string &direction) {
  }

  // Builds roof from polygon.
  virtual void build(utymap::math::Polygon &polygon) = 0;

 protected:

  /// Adds triangle to mesh from context.
  void addTriangle(const utymap::math::Vector3 &v0,
                   const utymap::math::Vector3 &v1,
                   const utymap::math::Vector3 &v2,
                   const utymap::math::Vector2 &u0,
                   const utymap::math::Vector2 &u1,
                   const utymap::math::Vector2 &u2) const {
    builderContext_.meshBuilder.addTriangle(meshContext_.mesh, v0, v1, v2, u0, u1, u2,
                                            meshContext_.geometryOptions, meshContext_.appearanceOptions);
  }

  const utymap::builders::BuilderContext &builderContext_;
  utymap::builders::MeshContext &meshContext_;
  double height_, minHeight_;
};

}
}

#endif // BUILDERS_BUILDINGS_ROOFS_ROOFBUILDER_HPP_DEFINED
