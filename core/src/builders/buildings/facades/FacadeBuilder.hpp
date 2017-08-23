#ifndef BUILDERS_BUILDINGS_FACADES_FACADEBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_FACADES_FACADEBUILDER_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/MeshContext.hpp"
#include "math/Polygon.hpp"

namespace utymap {
namespace builders {

/// Specifies base roof builder functionality.
class FacadeBuilder {
 public:
  FacadeBuilder(const utymap::builders::BuilderContext &builderContext,
                utymap::builders::MeshContext &meshContext) :
      builderContext_(builderContext),
      meshContext_(meshContext),
      height_(0),
      minHeight_(0) {
  }

  virtual ~FacadeBuilder() = default;

  /// Sets facade height.
  FacadeBuilder &setHeight(double height) {
    height_ = height;
    return *this;
  }

  /// Sets height above ground level.
  FacadeBuilder &setMinHeight(double minHeight) {
    minHeight_ = minHeight;
    return *this;
  }

  /// Sets color noise freq.
  FacadeBuilder &setColorNoiseFreq(double colorNoiseFreq) {
    meshContext_.appearanceOptions.colorNoiseFreq = colorNoiseFreq;
    return *this;
  }

  /// Builds roof from polygon.
  virtual void build(utymap::math::Polygon &polygon) = 0;

 protected:

  const utymap::builders::BuilderContext &builderContext_;
  utymap::builders::MeshContext &meshContext_;
  double height_, minHeight_;
};

}
}
#endif // BUILDERS_BUILDINGS_FACADES_FACADEBUILDER_HPP_DEFINED
