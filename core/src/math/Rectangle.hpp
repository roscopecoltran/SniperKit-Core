#ifndef MATH_RECTANGLE_HPP_DEFINED
#define MATH_RECTANGLE_HPP_DEFINED

#include "math/LineLinear.hpp"
#include "math/Vector2.hpp"

#include <algorithm>
#include <vector>

namespace utymap {
namespace math {

/// Represents axis aligned rectangle in 2D space.
struct Rectangle final {
  double xMin, xMax, yMin, yMax;

  LineLinear _left, _right, _bottom, _top;

  Rectangle()
      : Rectangle(std::numeric_limits<double>::max(),
                  std::numeric_limits<double>::max(),
                  std::numeric_limits<double>::min(),
                  std::numeric_limits<double>::min()) {
  }

  Rectangle(double xMin, double yMin, double xMax, double yMax) :
      xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax),
      _left(Vector2(xMin, yMin), Vector2(xMin, yMax)),
      _right(Vector2(xMax, yMin), Vector2(xMax, yMax)),
      _bottom(Vector2(xMin, yMin), Vector2(xMax, yMin)),
      _top(Vector2(xMin, yMax), Vector2(xMax, yMax)) {
  }

  void expand(const std::vector<Vector2> &contour) {
    for (const auto &p : contour) {
      expand(p);
    }
  }

  void expand(const Vector2 &point) {
    xMin = std::min(xMin, point.x);
    yMin = std::min(yMin, point.y);
    xMax = std::max(xMax, point.x);
    yMax = std::max(yMax, point.y);
  }

  bool contains(const Vector2 &pt) const {
    return ((pt.x >= xMin) && (pt.x <= xMax) && (pt.y >= yMin) && (pt.y <= yMax));
  }

  /// Checks whether point is on border of rectangle.
  bool isOnBorder(const Vector2 &point) const {
    return _left.contains(point) || _right.contains(point) ||
        _bottom.contains(point) || _top.contains(point);
  }

  double width() const {
    return xMax - xMin;
  }

  double height() const {
    return yMax - yMin;
  }
};

}
}
#endif //MATH_RECTANGLE_HPP_DEFINED
