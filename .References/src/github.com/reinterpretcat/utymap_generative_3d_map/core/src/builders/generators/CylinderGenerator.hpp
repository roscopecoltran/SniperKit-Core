#ifndef BUILDERS_GENERATORS_CYLINDERGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_CYLINDERGENERATOR_HPP_DEFINED

#include "builders/generators/AbstractGenerator.hpp"
#include "math/Quaternion.hpp"
#include "utils/MathUtils.hpp"

namespace utymap {
namespace builders {

/// Generates cylinder.
class CylinderGenerator : public AbstractGenerator {
 public:
  CylinderGenerator(const utymap::builders::BuilderContext &builderContext,
                    utymap::builders::MeshContext &meshContext) :
      AbstractGenerator(builderContext, meshContext) {
  }

  /// Sets center of cylinder.
  CylinderGenerator &setCenter(const utymap::math::Vector3 &center) {
    center_ = center;
    return *this;
  }

  /// Sets size of cylinder.
  CylinderGenerator &setSize(const utymap::math::Vector3 &size) {
    size1_ = size;
    size2_ = size;
    return *this;
  }

  /// Sets size of cylinder for first and last segments.
  CylinderGenerator &setSize(const utymap::math::Vector3 &size1,
                             const utymap::math::Vector3 &size2) {
    size1_ = size1;
    size2_ = size2;
    return *this;
  }

  /// Sets orientation of cylinder in 3D space.
  CylinderGenerator &setDirection(const utymap::math::Vector3 &direction,
                                  const utymap::math::Vector3 &right) {
    direction_ = direction;
    right_ = right;
    return *this;
  }

  /// Sets radial segment count.
  CylinderGenerator &setRadialSegments(int radialSegments) {
    radialSegments_ = radialSegments;
    return *this;
  }

  /// Sets max segment height.
  CylinderGenerator &setMaxSegmentHeight(double maxSegmentHeight) {
    maxSegmentHeight_ = maxSegmentHeight;
    return *this;
  }

  void generate() override {
    int heightSegments = maxSegmentHeight_!=0 ? static_cast<int>(std::ceil(size1_.y/maxSegmentHeight_)) : 1;
    double heightStep = size1_.y/heightSegments;
    double angleStep = 2*pi/radialSegments_;

    for (int j = 0; j < radialSegments_; ++j) {
      auto direction1 = utymap::math::Quaternion::fromAngleAxis(j*angleStep, direction_);
      auto direction2 =
          utymap::math::Quaternion::fromAngleAxis((j==radialSegments_ - 1 ? 0 : j + 1)*angleStep, direction_);

      for (int i = 0; i < heightSegments; i++) {
        auto radius1 = getRadius(static_cast<double>(heightSegments - i)/heightSegments);
        auto radius2 = getRadius(static_cast<double>(heightSegments - i - 1)/heightSegments);

        auto center1 = center_ + direction_*(heightStep*i);
        auto center2 = center_ + direction_*(heightStep*(i + 1));

        auto v0 = translate(center1 + scale(direction1*right_, radius1));
        auto v1 = translate(center1 + scale(direction2*right_, radius1));
        auto v2 = translate(center2 + scale(direction2*right_, radius2));
        auto v3 = translate(center2 + scale(direction1*right_, radius2));

        // add side.
        addTriangle(v1, v2, v0);
        addTriangle(v0, v2, v3);

        // add bottom cap part.
        if (i==0)
          addTriangle(translate(center1), v1, v0);
        // add top cap part.
        if (i==heightSegments - 1 && !isCone())
          addTriangle(translate(center2), v3, v2);
      }
    }
  }

 private:

  bool isCone() const {
    return size2_==utymap::math::Vector3::zero();
  }

  utymap::math::Vector3 getRadius(double value) const {
    return utymap::math::Vector3(utymap::utils::lerp(size2_.x, size1_.x, value),
                                 utymap::utils::lerp(size2_.y, size1_.y, value),
                                 utymap::utils::lerp(size2_.z, size1_.z, value));
  }

  utymap::math::Vector3 scale(const utymap::math::Vector3 &value, const utymap::math::Vector3 &size) const {
    /// NOTE use size.x for value.y to handle rotations
    return utymap::math::Vector3(value.x*size.x, value.y*size.x, value.z*size.z);
  }

  utymap::math::Vector3 center_ = utymap::math::Vector3::zero();
  utymap::math::Vector3 direction_ = utymap::math::Vector3::up();
  utymap::math::Vector3 right_ = utymap::math::Vector3::right();
  utymap::math::Vector3 size1_ = utymap::math::Vector3::zero();
  utymap::math::Vector3 size2_ = utymap::math::Vector3::zero();
  double maxSegmentHeight_ = 0;
  int radialSegments_ = 5;

};

}
}
#endif // BUILDERS_GENERATORS_CYLINDERGENERATOR_HPP_DEFINED
