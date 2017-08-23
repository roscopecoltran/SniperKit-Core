#ifndef MESHING_MESHBUILDER_HPP_DEFINED
#define MESHING_MESHBUILDER_HPP_DEFINED

#include "BoundingBox.hpp"
#include "QuadKey.hpp"
#include "heightmap/ElevationProvider.hpp"
#include "mapcss/ColorGradient.hpp"
#include "mapcss/StyleSheet.hpp"
#include "math/Polygon.hpp"
#include "math/Vector2.hpp"
#include "math/Vector3.hpp"
#include "math/Mesh.hpp"

#include <functional>

namespace utymap {
namespace builders {

/// Provides the way to build mesh in 3D space.
class MeshBuilder final {
 public:

  typedef const std::function<double(const utymap::GeoCoordinate &)> &EleInterpolator;

  struct GeometryOptions final {
    GeometryOptions(double area, double eleNoiseFreq, double elevation, double heightOffset, int segmentSplit = 0) :
        area(area),
        eleNoiseFreq(eleNoiseFreq),
        elevation(elevation),
        heightOffset(heightOffset),
        flipSide(false),
        hasBackSide(false),
        segmentSplit(segmentSplit) {
    }

    /// Max area of triangle in refined mesh.
    double area;

    /// Elevation noise frequency.
    double eleNoiseFreq;

    /// Fixed elevation. If specified elevation provider is not used.
    double elevation;

    /// Height offset.
    double heightOffset;

    /// If set then triangle side is flipped.
    bool flipSide;

    /// If set then backside should be generated too.
    bool hasBackSide;

    /// Flag indicating whether to suppress boundary segment splitting.
    ///     0 = split segments (default)
    ///     1 = no new vertices on the boundary
    ///     2 = prevent all segment splitting, including internal boundaries
    int segmentSplit;
  };

  struct AppearanceOptions final {
    /// Gradient used to calculate color at every vertex.
    const utymap::mapcss::ColorGradient &gradient;

    /// Color noise frequency.
    double colorNoiseFreq;

    /// Atlas id.
    std::uint16_t textureId;

    /// Texture coordinates map inside atlas.
    const utymap::mapcss::TextureRegion &textureRegion;

    /// Texture scale.
    double textureScale;

    AppearanceOptions(const utymap::mapcss::ColorGradient &gradient,
                      double colorNoiseFreq,
                      std::uint16_t textureId,
                      const utymap::mapcss::TextureRegion &textureRegion,
                      double textureScale) :
        gradient(gradient),
        colorNoiseFreq(colorNoiseFreq),
        textureId(textureId),
        textureRegion(textureRegion),
        textureScale(textureScale) {
    }
  };

  /// Creates builder with given elevation provider.
  MeshBuilder(const utymap::QuadKey &quadKey,
              const utymap::heightmap::ElevationProvider &eleProvider);

  ~MeshBuilder();

  /// Adds polygon to existing mesh using options provided.
  void addPolygon(utymap::math::Mesh &mesh,
                  utymap::math::Polygon &polygon,
                  const GeometryOptions &geometryOptions,
                  const AppearanceOptions &appearanceOptions) const;

  /// Adds simple plane to existing mesh using options provided.
  void addPlane(utymap::math::Mesh &mesh,
                const utymap::math::Vector2 &p1,
                const utymap::math::Vector2 &p2,
                const GeometryOptions &geometryOptions,
                const AppearanceOptions &appearanceOptions) const;

  /// Adds simple plane to existing mesh using elevation and options provided.
  void addPlane(utymap::math::Mesh &mesh,
                const utymap::math::Vector3 &p1,
                const utymap::math::Vector3 &p2,
                const GeometryOptions &geometryOptions,
                const AppearanceOptions &appearanceOptions) const;

  /// Adds triangle to mesh using options provided.
  void addTriangle(utymap::math::Mesh &mesh,
                   const utymap::math::Vector3 &v0,
                   const utymap::math::Vector3 &v1,
                   const utymap::math::Vector3 &v2,
                   const utymap::math::Vector2 &uv0,
                   const utymap::math::Vector2 &uv1,
                   const utymap::math::Vector2 &uv2,
                   const GeometryOptions &geometryOptions,
                   const AppearanceOptions &appearanceOptions) const;

  /// Writes texture mapping info into mesh.
  /// NOTE we want to support tiling with atlas textures. It requires to write some
  /// specific logic in shader. So, this code passes all information needed by it.
  /// This method exists because of performance optimization: you need to call it manually
  /// after all geometry related to one specific texture has been added.
  void writeTextureMappingInfo(utymap::math::Mesh &mesh,
                               const AppearanceOptions &appearanceOptions) const;

 private:

  static void addVertex(utymap::math::Mesh &mesh,
                        const utymap::math::Vector2 &p,
                        double ele, int color, int triIndex,
                        const utymap::math::Vector2 &uv);

  static void addVertex(utymap::math::Mesh &mesh,
                        const utymap::math::Vector3 &vertex,
                        int color, int triIndex,
                        const utymap::math::Vector2 &uv);

  const utymap::QuadKey quadKey_;
  const utymap::BoundingBox bbox_;
  const utymap::heightmap::ElevationProvider &eleProvider_;
};

}
}
#endif // MESHING_MESHBUILDER_HPP_DEFINED
