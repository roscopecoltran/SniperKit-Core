#ifndef BUILDERS_GENERATORS_TREEGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_TREEGENERATOR_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/MeshContext.hpp"
#include "builders/generators/CylinderGenerator.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"
#include "lsys/Turtle3d.hpp"

#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>

namespace utymap {
namespace builders {

/// Defines generator which generates a tree using lsystem.
class LSystemGenerator final : public utymap::lsys::Turtle3d {
  /// Maps word from lsystem grammar to the corresponding member function.
  static std::unordered_map<std::string, void (LSystemGenerator::*)()> WordMap;
 public:
  LSystemGenerator(const utymap::builders::BuilderContext &builderContext,
                   const utymap::mapcss::Style &style,
                   utymap::math::Mesh &mesh);

  /// Sets start geo position.
  LSystemGenerator &setPosition(const utymap::GeoCoordinate &coordinate, double height);

  void moveForward() override;

  void switchStyle() override;

  void say(const std::string &word) override;

 private:
  /// Adds sphere.
  void addSphere();

  /// Adds cylinder.
  void addCylinder();

  /// Adds cone.
  void addCone();

  /// Updates styles.
  void updateStyles();

  /// Maps given vertex to the cooresponding geocoordinate using position.
  utymap::math::Vector3 translate(const utymap::math::Vector3 &v) const;

  /// Gets size from current state.
  utymap::math::Vector3 getSize() const;

  const utymap::builders::BuilderContext &builderContext_;

  std::vector<utymap::builders::MeshBuilder::AppearanceOptions> appearances_;

  std::unique_ptr<utymap::builders::MeshContext> cylinderContext_;
  std::unique_ptr<utymap::builders::MeshContext> icoSphereContext_;

  utymap::builders::CylinderGenerator cylinderGenerator_;
  utymap::builders::IcoSphereGenerator icoSphereGenerator_;

  AbstractGenerator::TranslateFunc translationFunc_;

  utymap::GeoCoordinate position_;
  double minHeight_;
};

}
}

#endif // BUILDERS_GENERATORS_TREEGENERATOR_HPP_DEFINED
