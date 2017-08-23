#ifndef BUILDERS_GENERATORS_ICOSPHEREGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_ICOSPHEREGENERATOR_HPP_DEFINED

#include "builders/generators/AbstractGenerator.hpp"
#include "math/Vector3.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace utymap {
namespace builders {

/// Builds icosphere.
/// See http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
class IcoSphereGenerator final : public AbstractGenerator {
 public:
  IcoSphereGenerator(const utymap::builders::BuilderContext &builderContext,
                     utymap::builders::MeshContext &meshContext) :
      AbstractGenerator(builderContext, meshContext),
      center_(), size_(), recursionLevel_(0), isSemiSphere_(false) {
  }

  /// Sets center of icosphere.
  IcoSphereGenerator &setCenter(const utymap::math::Vector3 &center) {
    center_ = center;
    return *this;
  }

  /// Sets radius of icosphere.
  IcoSphereGenerator &setSize(const utymap::math::Vector3 &size) {
    size_ = size;
    return *this;
  }

  /// Sets recursion level.
  IcoSphereGenerator &setRecursionLevel(int recursionLevel) {
    recursionLevel_ = recursionLevel;
    return *this;
  }

  /// If true than only half will be generated.
  IcoSphereGenerator &isSemiSphere(bool value) {
    isSemiSphere_ = value;
    return *this;
  }

  void generate() override;

 private:
  ///  Returns index of point in the middle of p1 and p2.
  std::size_t getMiddlePoint(std::size_t p1, std::size_t p2);

  utymap::math::Vector3 scale(const utymap::math::Vector3 &v) const {
    return utymap::math::Vector3(v.x*size_.x, v.y*size_.y, v.z*size_.z);
  }

  utymap::math::Vector3 center_;
  utymap::math::Vector3 size_;
  int recursionLevel_;
  bool isSemiSphere_;

  std::unordered_map<std::uint64_t, std::size_t> middlePointIndexCache_;
  std::vector<utymap::math::Vector3> vertexList_;

};

}
}

#endif // BUILDERS_GENERATORS_ICOSPHEREGENERATOR_HPP_DEFINED
