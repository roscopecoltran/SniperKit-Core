#ifndef HEIGHTMAP_SRTMELEVATIONPROVIDER_HPP_DEFINED
#define HEIGHTMAP_SRTMELEVATIONPROVIDER_HPP_DEFINED

#include "BoundingBox.hpp"
#include "heightmap/ElevationProvider.hpp"
#include "utils/GeoUtils.hpp"

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <map>
#include <memory>
#include <mutex>
#include <iomanip>
#include <vector>

namespace utymap {
namespace heightmap {

/// Provides the way to get elevation for given location from SRTM data.
class SrtmElevationProvider final : public ElevationProvider {
  struct HgtCellKey {
    int lat, lon;

    HgtCellKey(int lat, int lon) : lat(lat), lon(lon) {}

    bool operator<(const HgtCellKey &other) const {
      return lat < other.lat || (!(other.lat < lat) && lon < other.lon);
    }
  };

  struct HgtCell {
    int totalPx, secondsPerPx, offset;
    std::vector<char> data;

    HgtCell(int totalPx, int secondsPerPx, std::vector<char> &data) :
        totalPx(totalPx),
        secondsPerPx(secondsPerPx),
        offset((totalPx*totalPx - totalPx)*2),
        data(std::move(data)) {
    }
  };

 public:

  SrtmElevationProvider(const std::string& indexPath, int maxCacheSize = 4) :
      dataPath_(indexPath + "data/"), maxCacheSize_(maxCacheSize) {
  }

  double getElevation(const utymap::QuadKey &quadKey, const utymap::GeoCoordinate &coordinate) const override {
    return getElevationImpl(quadKey, coordinate.latitude, coordinate.longitude);
  }

  double getElevation(const utymap::QuadKey &quadKey, double latitude, double longitude) const override {
    return getElevationImpl(quadKey, latitude, longitude);
  }

 private:

  void preload(const utymap::QuadKey &quadKey, HgtCellKey &hgtCellKey) const {
    std::lock_guard<std::mutex> lock(lock_);

    if (cells_.find(hgtCellKey)!=cells_.end())
      return;

    BoundingBox bbox = utymap::utils::GeoUtils::quadKeyToBoundingBox(quadKey);
    int minLat = static_cast<int>(bbox.minPoint.latitude);
    int minLon = static_cast<int>(bbox.minPoint.longitude);
    int maxLat = static_cast<int>(bbox.maxPoint.latitude);
    int maxLon = static_cast<int>(bbox.maxPoint.longitude);

    int latDiff = maxLat - minLat;
    int lonDiff = maxLon - minLon;

    for (int j = 0; j <= latDiff; j++)
      for (int i = 0; i <= lonDiff; i++) {
        HgtCellKey cellKey(minLat + j, minLon + i);

        if (cells_.find(cellKey)!=cells_.end())
          continue;

        std::string path = getFilePath(cellKey);
        HgtCell cell = readCell(path);
        cells_.emplace(cellKey, readCell(path));
      }
  }

  double getElevationImpl(const utymap::QuadKey &quadKey, double latitude, double longitude) const {
    int latDec = static_cast<int>(latitude);
    int lonDec = static_cast<int>(longitude);

    double secondsLat = (latitude - latDec)*3600;
    double secondsLon = (longitude - lonDec)*3600;

    HgtCellKey hgtCellKey(latDec, lonDec);
    auto cellPtr = cells_.find(hgtCellKey);
    if (cellPtr==cells_.end()) {
      preload(quadKey, hgtCellKey);
      cellPtr = cells_.find(hgtCellKey);
    }

    const auto &cell = cellPtr->second;

    // load tile
    //X corresponds to x/y values,
    //everything easter/norther (< S) is rounded to X.
    //
    //  y   ^
    //  3   |       |   S
    //      +-------+-------
    //  0   |   X   |
    //      +-------+-------->
    // (sec)    0        3   x  (lon)

    //both values are [0; totalPx - 1] (totalPx reserved for interpolating)
    int y = static_cast<int>(secondsLat/cell.secondsPerPx);
    int x = static_cast<int>(secondsLon/cell.secondsPerPx);

    //get norther and easter points
    int height2 = readPx(cell, y, x);
    int height0 = readPx(cell, y + 1, x);
    int height3 = readPx(cell, y, x + 1);
    int height1 = readPx(cell, y + 1, x + 1);

    //ratio where X lays
    double dy = std::fmod(secondsLat, cell.secondsPerPx)/cell.secondsPerPx;
    double dx = std::fmod(secondsLon, cell.secondsPerPx)/cell.secondsPerPx;

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

  static int readPx(const HgtCell &cell, int y, int x) {
    int pos = cell.offset + 2*(x - cell.totalPx*y);
    return (cell.data[pos]) << 8 | (cell.data[pos + 1]);
  }

  static HgtCell readCell(const std::string &path) {
    std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
    file.seekg(0, std::ios::end);
    auto size = file.tellg();

    int totalPx, secondsPerPx;
    switch (size) {
      case 1201*1201*2: // SRTM-3
        totalPx = 1201;
        secondsPerPx = 3;
        break;
      case 3601*3601*2: // SRTM-1
        totalPx = 3601;
        secondsPerPx = 1;
        break;
      default:file.close();
        throw std::domain_error(std::string("Cannot load srtm file:") + path);
    }

    file.seekg(0, std::ios::beg);

    std::vector<char> data((std::istreambuf_iterator<char>(file)),
                           (std::istreambuf_iterator<char>()));

    return HgtCell(totalPx, secondsPerPx, data);
  }

  std::string getFilePath(const HgtCellKey &key) const {
    std::ostringstream stream;
    stream << dataPath_
           << std::setfill('0')
           << (key.lat > 0 ? 'N' : 'S')
           << std::setw(2) << std::abs(key.lat) << std::setw(0)
           << (key.lon > 0 ? 'E' : 'W')
           << std::setw(3) << std::abs(key.lon)
           << ".hgt";
    return stream.str();
  }

  mutable std::map<HgtCellKey, HgtCell> cells_;
  mutable std::mutex lock_;
  std::string dataPath_;
  int maxCacheSize_;
};

}
}

#endif // HEIGHTMAP_SRTMELEVATIONPROVIDER_HPP_DEFINED
