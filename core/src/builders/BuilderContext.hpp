#ifndef BUILDERS_BUILDERCONTEXT_HPP_DEFINED
#define BUILDERS_BUILDERCONTEXT_HPP_DEFINED

#include "BoundingBox.hpp"
#include "CancellationToken.hpp"
#include "QuadKey.hpp"
#include "builders/MeshBuilder.hpp"
#include "heightmap/ElevationProvider.hpp"
#include "mapcss/StyleProvider.hpp"
#include <math/Mesh.hpp>
#include "utils/GeoUtils.hpp"

#include <functional>

namespace utymap {
namespace builders {

/// Provides the way to access all dependencies needed by various element builders.
struct BuilderContext final {
  typedef std::function<void(const utymap::math::Mesh &)> MeshCallback;
  typedef std::function<void(const utymap::entities::Element &)> ElementCallback;

  /// Current quadkey.
  const utymap::QuadKey quadKey;
  /// Bounding box if the quadkey.
  const utymap::BoundingBox boundingBox;
  /// Current style provider.
  const utymap::mapcss::StyleProvider &styleProvider;
  /// String table.
  utymap::index::StringTable &stringTable;
  /// Current elevation provider.
  const utymap::heightmap::ElevationProvider &eleProvider;
  /// Mesh callback should be called once mesh is constructed.
  std::function<void(const utymap::math::Mesh &)> meshCallback;
  /// Element callback is called to process original element by external logic.
  std::function<void(const utymap::entities::Element &)> elementCallback;
  /// Cancellation token.
  const utymap::CancellationToken &cancelToken;
  /// Mesh builder.
  const utymap::builders::MeshBuilder meshBuilder;

  BuilderContext(const utymap::QuadKey &quadKey,
                 const utymap::mapcss::StyleProvider &styleProvider,
                 utymap::index::StringTable &stringTable,
                 const utymap::heightmap::ElevationProvider &eleProvider,
                 const MeshCallback &meshCallback,
                 const ElementCallback &elementCallback,
                 const utymap::CancellationToken &cancelToken) :
      quadKey(quadKey),
      boundingBox(utymap::utils::GeoUtils::quadKeyToBoundingBox(quadKey)),
      styleProvider(styleProvider),
      stringTable(stringTable),
      eleProvider(eleProvider),
      meshCallback(meshCallback),
      elementCallback(elementCallback),
      cancelToken(cancelToken),
      meshBuilder(quadKey, eleProvider) {
  }
};

}
}
#endif // BUILDERS_BUILDERCONTEXT_HPP_DEFINED
