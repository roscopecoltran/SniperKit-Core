#ifndef HEIGHTMAP_GRIDELEVATIONPROVIDER_HPP_DEFINED
#define HEIGHTMAP_GRIDELEVATIONPROVIDER_HPP_DEFINED

#include "heightmap/ElevationProvider.hpp"
#include "utils/CoreUtils.hpp"
#include "utils/GeoUtils.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <map>
#include <mutex>
#include <vector>

namespace utymap {
namespace heightmap {

/// Provides the way to get elevation for given location from grid 
/// represented by comma separated list of integers
class GridElevationProvider final : public ElevationProvider {
  /// Holds elevation data information.
  struct EleData {
    int xStart = 0;
    int yStart = 0;
    int xStep = 0;
    int yStep = 0;
    int resolution = 0;

    std::vector<int> heights;
  };

  const double Scale = 1E7;

 public:
  GridElevationProvider(const std::string& indexPath) :
      dataPath_(indexPath + "data/") {
  }

  /// Gets elevation for given geocoordinate.
  double getElevation(const utymap::QuadKey &quadKey, const utymap::GeoCoordinate &coordinate) const override {
    return getElevation(quadKey, coordinate.latitude, coordinate.longitude);
  }

  /// Gets elevation for given geocoordinate.
  double getElevation(const utymap::QuadKey &quadKey, double latitude, double longitude) const override {
    auto data = data_.find(quadKey);
    if (data==data_.end()) {
      preload(quadKey);
      data = data_.find(quadKey);
    }

    int resolution = data->second.resolution;

    int x = static_cast<int>(longitude*Scale) - data->second.xStart;
    int y = static_cast<int>(latitude*Scale) - data->second.yStart;

    int x0 = clamp(x/data->second.xStep, 0, resolution);
    int y0 = clamp(y/data->second.yStep, 0, resolution);

    int x1 = std::min(x0 + 1, resolution);
    int y1 = std::min(y0 + 1, resolution);

    double dx = static_cast<double>(x - x0*data->second.xStep)/data->second.xStep;
    double dy = static_cast<double>(y - y0*data->second.yStep)/data->second.yStep;

    int cellSize = resolution + 1;
    int height2 = data->second.heights[x0 + y0*cellSize];
    int height0 = data->second.heights[x0 + y1*cellSize];
    int height3 = data->second.heights[x1 + y0*cellSize];
    int height1 = data->second.heights[x1 + y1*cellSize];

    // Bilinear interpolation
    // h0------------h1
    // |
    // |--dx-- .
    // |       |
    // |      dy
    // |       |
    // h2------------h3

    return height0*dy*(1 - dx) + height1*dy*(dx) + height2*(1 - dy)*(1 - dx) + height3*(1 - dy)*dx;
  }

 private:

  static int clamp(int n, int lower, int upper) {
    return std::max(lower, std::min(n, upper));
  }

  /// Preloads data for given quadkey.
  void preload(const utymap::QuadKey &quadKey) const {
    std::lock_guard<std::mutex> lock(lock_);

    if (data_.find(quadKey)!=data_.end())
      return;

    std::string filePath = getFilePath(quadKey);
    std::fstream file(filePath);
    if (!file.good())
      throw std::invalid_argument(std::string("Cannot find elevation file:") + filePath);

    EleData data;

    std::transform(std::istream_iterator<std::string>(file),
                   std::istream_iterator<std::string>(),
                   std::back_inserter(data.heights),
                   [&](const std::string &heightString) {
                     return utymap::utils::lexicalCast<int>(heightString);
                   });

    data.heights.shrink_to_fit();

    if (data.heights.empty())
      throw std::domain_error("Cannot get elevation data from:" + filePath);

    BoundingBox bbox = utymap::utils::GeoUtils::quadKeyToBoundingBox(quadKey);

    data.resolution = static_cast<int>(std::sqrt(data.heights.size())) - 1;
    data.xStart = static_cast<int>(bbox.minPoint.longitude*Scale);
    data.yStart = static_cast<int>(bbox.minPoint.latitude*Scale);
    data.xStep = static_cast<int>(bbox.width()/data.resolution*Scale);
    data.yStep = static_cast<int>(bbox.height()/data.resolution*Scale);

    data_.emplace(quadKey, std::move(data));
  }

  std::string getFilePath(const QuadKey &quadKey) const {
    std::stringstream ss;
    ss << dataPath_ << quadKey.levelOfDetail << "/" << utymap::utils::GeoUtils::quadKeyToString(quadKey) << ".ele";
    return ss.str();
  }

  mutable std::map<const QuadKey, EleData, QuadKey::Comparator> data_;
  mutable std::mutex lock_;
  const std::string dataPath_;
};

}
}

#endif // HEIGHTMAP_GRIDELEVATIONPROVIDER_HPP_DEFINED
