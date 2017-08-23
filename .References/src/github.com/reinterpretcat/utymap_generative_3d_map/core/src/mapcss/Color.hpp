#ifndef MAPCSS_COLOR_HPP_INCLUDED
#define MAPCSS_COLOR_HPP_INCLUDED

#include <algorithm>
#include <cstdint>

namespace utymap {
namespace mapcss {

/// Represents RGBA color.
struct Color final {
  unsigned char r, g, b, a;

  Color() : r(0), g(0), b(0), a(0) {
  }

  Color(int r, int g, int b, int a) :
      r(static_cast<unsigned char>(std::max(0, std::min(r, 0xff)))),
      g(static_cast<unsigned char>(std::max(0, std::min(g, 0xff)))),
      b(static_cast<unsigned char>(std::max(0, std::min(b, 0xff)))),
      a(static_cast<unsigned char>(std::max(0, std::min(a, 0xff)))) {
  };

  Color(int rgba) {
    r = static_cast<unsigned char>((rgba >> 24) & 0xff);
    g = static_cast<unsigned char>((rgba >> 16) & 0xff);
    b = static_cast<unsigned char>((rgba >> 8) & 0xff);
    a = static_cast<unsigned char>((rgba >> 0) & 0xff);
  }

  operator std::uint32_t() const {
    return (r << 24) | (g << 16) | (b << 8) | a;
  }

  Color operator+(const Color &o) const {
    return Color(r + o.r, g + o.g, b + o.b, a + o.a);
  }

  Color operator*(double f) const {
    return Color(static_cast<int>(r*f),
                 static_cast<int>(g*f),
                 static_cast<int>(b*f),
                 static_cast<int>(a*f));
  }
};

}
}

#endif  // MAPCSS_COLOR_HPP_INCLUDED
