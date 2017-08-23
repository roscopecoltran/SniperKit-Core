#ifndef BUILDERS_GENERATORS_WALLGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_WALLGENERATOR_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "builders/generators/AbstractGenerator.hpp"
#include "utils/GeoUtils.hpp"

#include <algorithm>
#include <vector>

namespace utymap {
namespace builders {

/// Provides the way to generate wall.
class WallGenerator : public AbstractGenerator {
 public:

  typedef std::vector<utymap::GeoCoordinate>::const_iterator Iterator;

  WallGenerator(const utymap::builders::BuilderContext &builderContext,
                utymap::builders::MeshContext &meshContext) :
      AbstractGenerator(builderContext, meshContext),
      begin_(), end_(), width_(0), height_(0), heightOffset_(0), length_(0), gap_(0) {
  }

  WallGenerator &setGeometry(Iterator begin, Iterator end) {
    begin_ = begin;
    end_ = end;
    return *this;
  }

  WallGenerator &setWidth(double width) {
    width_ = width;
    return *this;
  }

  WallGenerator &setHeight(double height) {
    height_ = height;
    return *this;
  }

  WallGenerator &setHeightOffset(double offset) {
    heightOffset_ = offset;
    return *this;
  }

  WallGenerator &setLength(double length) {
    length_ = length;
    return *this;
  }

  WallGenerator &setGap(double gap) {
    gap_ = gap;
    return *this;
  }

  void generate() override {
    if (begin_==end_) return;

    auto size = static_cast<std::size_t>(std::distance(begin_, end_) - 1);
    auto fullLength = length_ + gap_;
    auto ratio = length_/fullLength;
    for (std::size_t i = 0; i < size; ++i) {
      const auto &p0 = *(begin_ + i);
      const auto &p1 = *(begin_ + i + 1);

      double distanceInMeters = utymap::utils::GeoUtils::distance(p0, p1);
      int count = std::max(static_cast<int>(distanceInMeters/fullLength), 1);

      auto start = p0;
      for (int j = 1; j <= count; ++j) {
        double offset = static_cast<double>(j)/count;
        auto end = count==1 ? p1 : utymap::utils::GeoUtils::newPoint(p0, p1, offset);
        buildSegment(utymap::math::Vector2(start.longitude, start.latitude),
                     utymap::math::Vector2(start.longitude + (end.longitude - start.longitude)*ratio,
                                           start.latitude + (end.latitude - start.latitude)*ratio));
        start = end;
      }
    }

    builderContext_.meshBuilder.writeTextureMappingInfo(meshContext_->mesh,
                                                        meshContext_->appearanceOptions);
  }

 private:

  /// Builds segment separately. NOTE better to connect them.
  void buildSegment(const utymap::math::Vector2 &p0, const utymap::math::Vector2 &p1) const {
    // get segments.
    auto direction = (p1 - p0).normalized();
    utymap::math::Vector2 normal(-direction.y, direction.x);
    auto leftP0 = p0 + normal*width_;
    auto leftP1 = p1 + normal*width_;
    auto rightP0 = p0 - normal*width_;
    auto rightP1 = p1 - normal*width_;

    auto bottomLeftP0 =
        heightOffset_ + builderContext_.eleProvider.getElevation(builderContext_.quadKey, leftP0.y, leftP0.x);
    auto bottomLeftP1 =
        heightOffset_ + builderContext_.eleProvider.getElevation(builderContext_.quadKey, leftP1.y, leftP1.x);
    auto bottomRightP0 =
        heightOffset_ + builderContext_.eleProvider.getElevation(builderContext_.quadKey, rightP0.y, rightP0.x);
    auto bottomRightP1 =
        heightOffset_ + builderContext_.eleProvider.getElevation(builderContext_.quadKey, rightP1.y, rightP1.x);

    buildParallelepiped(utymap::math::Vector3(leftP0.x, bottomLeftP0, leftP0.y),
                        utymap::math::Vector3(leftP1.x, bottomLeftP1, leftP1.y),
                        utymap::math::Vector3(rightP0.x, bottomRightP0, rightP0.y),
                        utymap::math::Vector3(rightP1.x, bottomRightP1, rightP1.y),
                        height_);
  }

  Iterator begin_, end_;
  double width_, height_, heightOffset_, length_, gap_;
};

}
}

#endif // BUILDERS_GENERATORS_WALLGENERATOR_HPP_DEFINED
