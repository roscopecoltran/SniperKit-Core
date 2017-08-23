#ifndef APPLICATION_HPP_DEFINED
#define APPLICATION_HPP_DEFINED

#include "BoundingBox.hpp"
#include "CancellationToken.hpp"
#include "QuadKey.hpp"
#include "LodRange.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/CacheBuilder.hpp"
#include "builders/MeshCache.hpp"
#include "builders/QuadKeyBuilder.hpp"
#include "builders/buildings/BuildingBuilder.hpp"
#include "builders/misc/BarrierBuilder.hpp"
#include "builders/misc/LampBuilder.hpp"
#include "builders/poi/TreeBuilder.hpp"
#include "builders/terrain/TerraBuilder.hpp"
#include "heightmap/FlatElevationProvider.hpp"
#include "heightmap/GridElevationProvider.hpp"
#include "heightmap/SrtmElevationProvider.hpp"
#include "index/GeoStore.hpp"
#include "index/InMemoryElementStore.hpp"
#include "index/PersistentElementStore.hpp"
#include "mapcss/MapCssParser.hpp"
#include "mapcss/StyleSheet.hpp"
#include "utils/CoreUtils.hpp"

#include "Callbacks.hpp"
#include "ExportElementVisitor.hpp"

#include <exception>
#include <fstream>
#include <memory>
#include <vector>

/// Exposes API for external usage.
class Application {
 public:
  enum class ElevationDataType { Flat = 0, Srtm, Grid };

  /// Composes object graph.
  explicit Application(const char *indexPath) :
      indexPath_(indexPath),
      stringTable_(indexPath),
      geoStore_(stringTable_),
      flatEleProvider_(), srtmEleProvider_(indexPath), gridEleProvider_(indexPath),
      quadKeyBuilder_(geoStore_, stringTable_) {
    registerDefaultBuilders();
  }

  /// Registers stylesheet.
  void registerStylesheet(const char *path, OnNewDirectory *directoryCallback) {
    auto &styleProvider = getStyleProvider(path);
    std::string root = indexPath_ + "cache/" + styleProvider.getTag() + '/';
    createDataDirs(root, directoryCallback);
  }

  /// Registers new in-memory store.
  void registerInMemoryStore(const char *key) {
    geoStore_.registerStore(key, utymap::utils::make_unique<utymap::index::InMemoryElementStore>(stringTable_));
  }

  /// Registers new persistent store.
  void registerPersistentStore(const char *key, const char *dataPath, OnNewDirectory *directoryCallback) {
    geoStore_
        .registerStore(key, utymap::utils::make_unique<utymap::index::PersistentElementStore>(dataPath, stringTable_));
    createDataDirs(indexPath_ + "data/", directoryCallback);
  }

  /// Enables or disables mesh caching.
  void enableMeshCache(bool enabled) {
    for (const auto &entry : meshCaches_) {
      if (enabled) entry.second->enable();
      else entry.second->disable();
    }
  }

  /// Adds data to store.
  void addToStore(const char *key,
                  const char *styleFile,
                  const char *path,
                  const utymap::QuadKey &quadKey,
                  OnError *errorCallback) {
    safeExecute([&]() {
      geoStore_.add(key, path, quadKey, getStyleProvider(styleFile));
    }, errorCallback);
  }

  /// Adds data to store.
  void addToStore(const char *key,
                  const char *styleFile,
                  const char *path,
                  const utymap::BoundingBox &bbox,
                  const utymap::LodRange &range,
                  OnError *errorCallback) {
    safeExecute([&]() {
      geoStore_.add(key, path, bbox, range, getStyleProvider(styleFile));
    }, errorCallback);
  }

  /// Adds data to store.
  void addToStore(const char *key,
                  const char *styleFile,
                  const char *path,
                  const utymap::LodRange &range,
                  OnError *errorCallback) {
    safeExecute([&]() {
      geoStore_.add(key, path, range, getStyleProvider(styleFile));
    }, errorCallback);
  }

  /// Adds element to store.
  void addToStore(const char *key,
                  const char *styleFile,
                  const utymap::entities::Element &element,
                  const utymap::LodRange &range,
                  OnError *errorCallback) {
    safeExecute([&]() {
      geoStore_.add(key, element, range, getStyleProvider(styleFile));
    }, errorCallback);
  }

  bool hasData(const utymap::QuadKey &quadKey) const {
    return geoStore_.hasData(quadKey);
  }

  /// Loads given quadKey.
  void loadQuadKey(int tag,
                   const char *styleFile,
                   const utymap::QuadKey &quadKey,
                   const ElevationDataType &eleDataType,
                   OnMeshBuilt *meshCallback,
                   OnElementLoaded *elementCallback,
                   OnError *errorCallback,
                   utymap::CancellationToken *cancellationToken) {
    safeExecute([&]() {
      auto &styleProvider = getStyleProvider(styleFile);
      auto &eleProvider = getElevationProvider(quadKey, eleDataType);
      ExportElementVisitor elementVisitor(tag, quadKey, stringTable_, styleProvider, eleProvider, elementCallback);
      quadKeyBuilder_.build(
          quadKey, styleProvider, eleProvider,
          [&meshCallback, tag](const utymap::math::Mesh &mesh) {
            // NOTE do not notify if mesh is empty.
            if (!mesh.vertices.empty()) {
              meshCallback(tag, mesh.name.data(),
                           mesh.vertices.data(), static_cast<int>(mesh.vertices.size()),
                           mesh.triangles.data(), static_cast<int>(mesh.triangles.size()),
                           mesh.colors.data(), static_cast<int>(mesh.colors.size()),
                           mesh.uvs.data(), static_cast<int>(mesh.uvs.size()),
                           mesh.uvMap.data(), static_cast<int>(mesh.uvMap.size()));
            }
          }, [&elementVisitor](const utymap::entities::Element &element) {
            element.accept(elementVisitor);
          }, *cancellationToken);
    }, errorCallback);
  }

  /// Gets id for the string.
  std::uint32_t getStringId(const char *str) const {
    return stringTable_.getId(str);
  }

  /// Gets elevation for given geocoordinate using specific elevation provider.
  double getElevation(const utymap::QuadKey &quadKey,
                      const ElevationDataType &elevationDataType,
                      const utymap::GeoCoordinate &coordinate) const {
    return getElevationProvider(quadKey, elevationDataType).getElevation(quadKey, coordinate);
  }

 private:

  static void safeExecute(const std::function<void()> &action, OnError *errorCallback) {
    try {
      action();
    }
    catch (std::exception &ex) {
      errorCallback(ex.what());
    }
  }

  const utymap::heightmap::ElevationProvider &getElevationProvider(const utymap::QuadKey &quadKey,
                                                                   const ElevationDataType &eleDataType) const {
    switch (eleDataType) {
      case ElevationDataType::Grid: return gridEleProvider_;
      case ElevationDataType::Srtm: return srtmEleProvider_;
      default: return flatEleProvider_;
    }
  }

  const utymap::mapcss::StyleProvider &getStyleProvider(const std::string &stylePath) {
    auto pair = styleProviders_.find(stylePath);
    if (pair!=styleProviders_.end())
      return *pair->second;

    std::ifstream styleFile(stylePath);
    if (!styleFile.good())
      throw std::invalid_argument(std::string("Cannot read mapcss file:") + stylePath);

    // NOTE not safe, but don't want to use boost filesystem only for this task.
    std::string dir = stylePath.substr(0, stylePath.find_last_of("\\/") + 1);
    utymap::mapcss::MapCssParser parser(dir);
    utymap::mapcss::StyleSheet stylesheet = parser.parse(styleFile);

    styleProviders_.emplace(
        stylePath,
        utymap::utils::make_unique<const utymap::mapcss::StyleProvider>(stylesheet, stringTable_));

    return *styleProviders_[stylePath];
  }

  void registerDefaultBuilders() {
    registerBuilder<utymap::builders::TerraBuilder>("terrain", true);
    registerBuilder<utymap::builders::BuildingBuilder>("building");
    registerBuilder<utymap::builders::TreeBuilder>("tree");
    registerBuilder<utymap::builders::BarrierBuilder>("barrier");
    registerBuilder<utymap::builders::LampBuilder>("lamp");
  }

  template<typename Builder>
  void registerBuilder(const std::string &name, bool useCache = false) {
    if (useCache)
      meshCaches_.emplace(name, utymap::utils::make_unique<utymap::builders::MeshCache>(indexPath_, name));

    quadKeyBuilder_
        .registerElementBuilder(name, useCache ? createCacheFactory<Builder>(name) : createFactory<Builder>());
  }

  template<typename Builder>
  utymap::builders::QuadKeyBuilder::ElementBuilderFactory createFactory() const {
    return [](const utymap::builders::BuilderContext &context) {
      return utymap::utils::make_unique<Builder>(context);
    };
  }

  template<typename Builder>
  utymap::builders::QuadKeyBuilder::ElementBuilderFactory createCacheFactory(const std::string &name) const {
    auto &meshCache = *meshCaches_.find(name)->second;
    return [&, name](const utymap::builders::BuilderContext &context) {
      return utymap::utils::make_unique<utymap::builders::CacheBuilder<Builder>>(meshCache, context);
    };
  }

  static void createDataDirs(const std::string &root, OnNewDirectory *directoryCallback) {
    const int MinLevelOfDetail = 1;
    const int MaxLevelOfDetail = 16;
    for (int i = MinLevelOfDetail; i <= MaxLevelOfDetail; ++i) {
      auto lodDir = root + utymap::utils::toString(i);
      directoryCallback(lodDir.c_str());
    }
  }

  std::string indexPath_;
  utymap::index::StringTable stringTable_;
  utymap::index::GeoStore geoStore_;

  utymap::heightmap::FlatElevationProvider flatEleProvider_;
  utymap::heightmap::SrtmElevationProvider srtmEleProvider_;
  utymap::heightmap::GridElevationProvider gridEleProvider_;

  utymap::builders::QuadKeyBuilder quadKeyBuilder_;
  std::unordered_map<std::string, std::unique_ptr<utymap::builders::MeshCache>> meshCaches_;
  std::unordered_map<std::string, std::unique_ptr<const utymap::mapcss::StyleProvider>> styleProviders_;
};

#endif // APPLICATION_HPP_DEFINED
