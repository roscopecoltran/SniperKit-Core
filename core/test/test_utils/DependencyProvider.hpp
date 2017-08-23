#ifndef TEST_DEPENDENCYPROVIDER_HPP_DEFINED
#define TEST_DEPENDENCYPROVIDER_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "heightmap/FlatElevationProvider.hpp"
#include "index/StringTable.hpp"
#include "mapcss/MapCssParser.hpp"
#include "mapcss/StyleProvider.hpp"

#include <cstdio>
#include <iostream>

namespace utymap {
namespace tests {

class DependencyProvider {
 public:

  DependencyProvider() : cancelToken() {}

  ~DependencyProvider() {
    stringTable_.reset();

    bool hasError = std::remove("string.idx") > 0;
    hasError = std::remove("string.dat") > 0 || hasError;

    if (hasError)
      std::cout << "Error while deleting index files.";
  }

  std::shared_ptr<utymap::index::StringTable> getStringTable() {
    if (stringTable_==nullptr) {
      stringTable_ = std::make_shared<utymap::index::StringTable>("");
    }
    return stringTable_;
  }

  std::shared_ptr<utymap::heightmap::FlatElevationProvider> getElevationProvider() {
    if (eleProvider_==nullptr) {
      eleProvider_ = std::make_shared<utymap::heightmap::FlatElevationProvider>();
    }
    return eleProvider_;
  }

  std::shared_ptr<utymap::mapcss::StyleProvider> getStyleProvider(const std::string &stylesheetStr) {
    auto stylesheet = utymap::mapcss::MapCssParser().parse(stylesheetStr);
    return getStyleProvider(stylesheet);
  }

  std::shared_ptr<utymap::mapcss::StyleProvider> getStyleProvider(const utymap::mapcss::StyleSheet &stylesheet) {
    if (styleProvider_==nullptr) {
      utymap::mapcss::MapCssParser parser;
      styleProvider_ = std::make_shared<utymap::mapcss::StyleProvider>(stylesheet, *getStringTable());
    }
    return styleProvider_;
  }

  std::shared_ptr<utymap::mapcss::StyleProvider> getStyleProvider() const {
    if (styleProvider_==nullptr)
      throw std::invalid_argument("styleProvider");
    return styleProvider_;
  }

  std::shared_ptr<utymap::builders::BuilderContext> createBuilderContext(
      const utymap::QuadKey &quadKey,
      const std::string &stylesheet,
      std::function<void(const utymap::math::Mesh &)> meshCallback = nullptr,
      std::function<void(const utymap::entities::Element &)> elementCallback = nullptr) {
    return std::make_shared<utymap::builders::BuilderContext>(
        quadKey,
        *getStyleProvider(stylesheet),
        *getStringTable(),
        *getElevationProvider(),
        meshCallback,
        elementCallback,
        cancelToken);
  }

  const utymap::CancellationToken &getCancellationToken() const {
    return cancelToken;
  }

 private:
  std::shared_ptr<utymap::heightmap::FlatElevationProvider> eleProvider_;
  std::shared_ptr<utymap::index::StringTable> stringTable_;
  std::shared_ptr<utymap::mapcss::StyleProvider> styleProvider_;
  const utymap::CancellationToken cancelToken;
};

}
}

#endif  // TEST_DEPENDENCYPROVIDER_HPP_DEFINED
