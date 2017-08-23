#ifndef BUILDERS_TERRAIN_TERRAEXTRAS_HPP_DEFINED
#define BUILDERS_TERRAIN_TERRAEXTRAS_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "math/Mesh.hpp"

namespace utymap {
namespace builders {

/// Provides the way to extend terrain mesh with additional data.
class TerraExtras final {
 public:
  /// Specifies mesh region which should be used.
  struct Context {
    std::size_t startVertex;
    std::size_t startTriangle;
    std::size_t startColor;

    utymap::math::Mesh &mesh;
    const utymap::mapcss::Style &style;

    Context(utymap::math::Mesh &mesh,
            const utymap::mapcss::Style &style) :
        startVertex(mesh.vertices.size()),
        startTriangle(mesh.triangles.size()),
        startColor(mesh.colors.size()),
        mesh(mesh), style(style) {
    }
  };

  /// Specifies Extras function signature.
  typedef std::function<void(const utymap::builders::BuilderContext &, TerraExtras::Context &)> ExtrasFunc;

  /// Extends mesh with trees.
  static void addForest(const utymap::builders::BuilderContext &builderContext, TerraExtras::Context &extrasContext);

  /// Extends mesh with water surface.
  static void addWater(const utymap::builders::BuilderContext &builderContext, TerraExtras::Context &extrasContext);
};

}
}
#endif // BUILDERS_TERRAIN_TERRAEXTRAS_HPP_DEFINED
