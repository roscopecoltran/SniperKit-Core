#ifndef BUILDERS_TERRAIN_SURFACEGENERATOR_HPP_DEFINED
#define BUILDERS_TERRAIN_SURFACEGENERATOR_HPP_DEFINED

#include "builders/terrain/TerraExtras.hpp"
#include "builders/terrain/TerraGenerator.hpp"
#include "math/Mesh.hpp"
#include "math/Polygon.hpp"
#include "math/Vector2.hpp"

namespace utymap {
namespace builders {

/// Provides the way to generate terrain mesh.
class SurfaceGenerator final : public TerraGenerator {
 public:
  SurfaceGenerator(const BuilderContext &context,
                   const utymap::mapcss::Style &style,
                   const ClipperLib::Path &tileRect);

  void onNewRegion(const std::string &type,
                   const utymap::entities::Element &element,
                   const utymap::mapcss::Style &style,
                   const std::shared_ptr<Region> &region) override;

  void generateFrom(const std::vector<Layer> &layers) override;

 protected:
  /// Adds geometry to mesh.
  void addGeometry(int level, utymap::math::Polygon &polygon, const RegionContext &regionContext) override;

 private:
  /// Builds foreground surface.
  void buildForeground(const std::vector<Layer> &layers);

  /// Builds background surface.
  void buildBackground();

  /// Builds layer.
  void buildLayer(const Layer &layer);

  /// Builds mesh using paths data.
  void buildRegion(const Region &region);

  /// Adds extras to mesh, e.g. trees, water surface if meshExtras are specified in options.
  void addExtrasIfNecessary(utymap::math::Mesh &mesh,
                            TerraExtras::Context &extrasContext,
                            const RegionContext &regionContext) const;

  ClipperLib::ClipperEx foregroundClipper_;
  ClipperLib::ClipperEx backgroundClipper_;
};

}
}
#endif // BUILDERS_TERRAIN_SURFACEGENERATOR_HPP_DEFINED
