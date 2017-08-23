#ifndef BUILDERS_TERRAIN_EXTERIORGENERATOR_HPP_DEFINED
#define BUILDERS_TERRAIN_EXTERIORGENERATOR_HPP_DEFINED

#include "builders/terrain/TerraGenerator.hpp"

#include <memory>

namespace utymap {
namespace builders {

/// Generates meshes outside terrain surface, e.g. tunnels, roads above ground, etc.
class ExteriorGenerator final : public TerraGenerator {
 public:
  ExteriorGenerator(const BuilderContext &context,
                    const utymap::mapcss::Style &style,
                    const ClipperLib::Path &tileRect);

  void onNewRegion(const std::string &type,
                   const utymap::entities::Element &element,
                   const utymap::mapcss::Style &style,
                   const std::shared_ptr<Region> &region) override;

  void generateFrom(const std::vector<Layer> &layers) override;

  ~ExteriorGenerator();

 protected:
  void addGeometry(int level, utymap::math::Polygon &polygon, const RegionContext &regionContext) override;

 private:
  class ExteriorGeneratorImpl;
  std::unique_ptr<ExteriorGeneratorImpl> p_impl;
};

}
}

#endif // BUILDERS_TERRAIN_EXTERIORGENERATOR_HPP_DEFINED
