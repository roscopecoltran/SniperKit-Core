#ifndef MAPCSS_COLORGRADIENT_HPP_INCLUDED
#define MAPCSS_COLORGRADIENT_HPP_INCLUDED

#include "mapcss/Color.hpp"

#include <cstdint>
#include <vector>
#include <utility>

namespace utymap {
namespace mapcss {

/// Represents color gradient.
class ColorGradient final {
 public:

  /// gradient data: first - time, second - color.
  typedef std::vector<std::pair<double, utymap::mapcss::Color>> GradientData;

  ColorGradient() {}

  explicit ColorGradient(const GradientData &colors) :
      colors_(colors) {
  }

  ColorGradient(ColorGradient &&other) :
      colors_(std::move(other.colors_)) {
  }

  ColorGradient &operator=(ColorGradient &&other) {
    if (this!=&other)
      colors_ = std::move(other.colors_);

    return *this;
  }

  utymap::mapcss::Color evaluate(double time) const {
    if (colors_.empty())
      return utymap::mapcss::Color();

    GradientData::size_type index = 0;
    while (index < colors_.size() - 1 && colors_[index].first < time)
      index++;

    auto pairA = colors_[index!=0 ? index - 1 : 0];
    auto pairB = colors_[index];

    double timeA = pairA.first;
    double timeB = pairB.first;
    double mu = index==0 ? 0 : (time - timeA)/(timeB - timeA);

    return interpolate(pairA.second, pairB.second, mu);
  }

  /// Returns true if there is no color specified.
  bool empty() const { return colors_.empty(); }

 private:

  /// So far, use linear interpolation algorithm as the fastest.
  static utymap::mapcss::Color interpolate(const utymap::mapcss::Color &a,
                                           const utymap::mapcss::Color &b,
                                           double r) {
    return a*(1.0 - r) + b*r;
  }

  GradientData colors_;
};

}
}

#endif  // MAPCSS_COLORGRADIENT_HPP_INCLUDED
