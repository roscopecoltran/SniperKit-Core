#ifndef UTILS_GRADIENTUTILS_HPP_DEFINED
#define UTILS_GRADIENTUTILS_HPP_DEFINED

#include "mapcss/Color.hpp"
#include "mapcss/Style.hpp"
#include "mapcss/ColorGradient.hpp"
#include "mapcss/StyleProvider.hpp"
#include "utils/NoiseUtils.hpp"

#include <cstdint>
#include <string>
#include <regex>
#include <memory>

namespace utymap {
namespace utils {

/// Provides the way to work with color gradient and color data.
class GradientUtils final {
 public:

  /// Parses color from string.
  static utymap::mapcss::Color parseColor(const std::string &colorStr);

  /// Checks whether string is gradient.
  static bool isGradient(const std::string &gradientStr);

  /// Parses color gradient from string.
  static std::unique_ptr<const utymap::mapcss::ColorGradient> parseGradient(const std::string &gradientStr);

  /// Gets gradient.
  static const utymap::mapcss::ColorGradient &evaluateGradient(const utymap::mapcss::StyleProvider &styleProvider,
                                                               const utymap::mapcss::Style &style,
                                                               const std::string &key);

  /// Gets color for specific coordinate using coherent noise function
  static utymap::mapcss::Color getColor(const utymap::mapcss::ColorGradient &gradient,
                                        double x, double y, double noise) {
    double colorTime = (NoiseUtils::perlin2D(x, y, noise) + 1)/2;
    return gradient.evaluate(colorTime);
  }

 private:
  static const std::regex gradientRegEx;
};

}
}

#endif // UTILS_GRADIENTUTILS_HPP_DEFINED
