#ifndef BUILDERS_GENERATORS_ABSTRACTGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_ABSTRACTGENERATOR_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/MeshContext.hpp"
#include "utils/NoiseUtils.hpp"

#include <functional>

namespace utymap {
namespace builders {

/// Specifies basic behaviour of mesh generators.
class AbstractGenerator {
 public:
  typedef std::function<utymap::math::Vector3(const utymap::math::Vector3 &)> TranslateFunc;

  AbstractGenerator(const utymap::builders::BuilderContext &builderContext,
                    utymap::builders::MeshContext &meshContext) :
      builderContext_(builderContext),
      meshContext_(&meshContext),
      vertNoiseFreq_(0),
      translateFunc_(nullptr) {
  }

  virtual ~AbstractGenerator() = default;

  /// Generates mesh data and updates given mesh.
  virtual void generate() = 0;

  /// Changes mesh context to given.
  AbstractGenerator &setContext(utymap::builders::MeshContext &meshContext) {
    meshContext_ = &meshContext;
    return *this;
  }

  /// Sets vertex noise frequency
  AbstractGenerator &setVertexNoiseFreq(double vertNoiseFreq) {
    vertNoiseFreq_ = vertNoiseFreq;
    return *this;
  }

  /// Sets color noise frequency
  AbstractGenerator &setColorNoiseFreq(double colorNoiseFreq) {
    meshContext_->appearanceOptions.colorNoiseFreq = colorNoiseFreq;
    return *this;
  }

  /// Sets translation function used to translate coordinates.
  AbstractGenerator &setTranslation(TranslateFunc &translateFunc) {
    translateFunc_ = &translateFunc;
    return *this;
  }

 protected:

  /// Translate given coordinate if translation function is present.
  utymap::math::Vector3 translate(const utymap::math::Vector3 &v) const {
    return translateFunc_!=nullptr ? (*translateFunc_)(v) : v;
  }

  /// Adds triangle to mesh.
  void addTriangle(const utymap::math::Vector3 &v0,
                   const utymap::math::Vector3 &v1,
                   const utymap::math::Vector3 &v2) const {
    double noise = std::abs(vertNoiseFreq_) > 1E-5
                   ? utymap::utils::NoiseUtils::perlin2D(v0.x, v0.z, vertNoiseFreq_)
                   : 0;

    builderContext_.meshBuilder.addTriangle(meshContext_->mesh,
                                            utymap::math::Vector3(v0.x, v0.y + noise, v0.z),
                                            utymap::math::Vector3(v1.x, v1.y + noise, v1.z),
                                            utymap::math::Vector3(v2.x, v2.y + noise, v2.z),
                                            utymap::math::Vector2(0, 0),
                                            utymap::math::Vector2(1, 0),
                                            utymap::math::Vector2(1, 1),
                                            meshContext_->geometryOptions,
                                            meshContext_->appearanceOptions);
  }

  void addPlane(const utymap::math::Vector2 &v0,
                const utymap::math::Vector2 &v1,
                double bottom,
                double top) const {
    meshContext_->geometryOptions.elevation = bottom;
    meshContext_->geometryOptions.heightOffset = top - bottom;

    builderContext_.meshBuilder.addPlane(meshContext_->mesh, v0, v1,
                                         meshContext_->geometryOptions, meshContext_->appearanceOptions);
  }

  void addPlane(const utymap::math::Vector3 &v0,
                const utymap::math::Vector3 &v1,
                double heightOffset) const {
    meshContext_->geometryOptions.heightOffset = heightOffset;

    builderContext_.meshBuilder.addPlane(meshContext_->mesh, v0, v1,
                                         meshContext_->geometryOptions, meshContext_->appearanceOptions);
  }

  /// Builds parallelepiped with two sides on different height.
  void buildParallelepiped(const utymap::math::Vector3 &leftP0,
                           const utymap::math::Vector3 &leftP1,
                           const utymap::math::Vector3 &rightP0,
                           const utymap::math::Vector3 &rightP1,
                           double height) const {
    addPlane(leftP1, leftP0, height);
    addPlane(rightP1, leftP1, height);
    addPlane(rightP0, rightP1, height);
    addPlane(leftP0, rightP0, height);

    addTriangle(utymap::math::Vector3(leftP0.x, leftP0.y + height, leftP0.z),
                utymap::math::Vector3(leftP1.x, leftP1.y + height, leftP1.z),
                utymap::math::Vector3(rightP0.x, rightP0.y + height, rightP0.z));

    addTriangle(utymap::math::Vector3(rightP0.x, rightP0.y + height, rightP0.z),
                utymap::math::Vector3(leftP1.x, leftP1.y + height, leftP1.z),
                utymap::math::Vector3(rightP1.x, rightP1.y + height, rightP1.z));
  }

  const utymap::builders::BuilderContext &builderContext_;
  utymap::builders::MeshContext *meshContext_;

 private:
  double vertNoiseFreq_;
  TranslateFunc *translateFunc_;
};

}
}

#endif // BUILDERS_GENERATORS_ABSTRACTGENERATOR_HPP_DEFINED
