#ifndef INDEX_LODRANGE_HPP_DEFINED
#define INDEX_LODRANGE_HPP_DEFINED

#include "utils/GeoUtils.hpp"

#include <stdexcept>

namespace utymap {

/// Represents level of details range.
struct LodRange final {
  int start;
  int end;

  LodRange(int s, int e) : start(s), end(e) {
    int min = utymap::utils::GeoUtils::MinLevelOfDetails;
    int max = utymap::utils::GeoUtils::MaxLevelOfDetails;

    if (s < min || s > max || e < min || e > max)
      throw std::domain_error("Unsupported level of details");
  }
};

}

#endif // INDEX_LODRANGE_HPP_DEFINED
