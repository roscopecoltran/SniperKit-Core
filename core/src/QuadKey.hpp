#ifndef QUADKEY_HPP_DEFINED
#define QUADKEY_HPP_DEFINED

namespace utymap {

/// Represents quadkey: a node of quadtree
struct QuadKey final {
  struct Comparator {
    bool operator()(const QuadKey &lhs, const QuadKey &rhs) const {
      if (lhs.levelOfDetail==rhs.levelOfDetail) {
        if (lhs.tileX==rhs.tileX) {
          return lhs.tileY < rhs.tileY;
        }
        return lhs.tileX < rhs.tileX;
      }
      return lhs.levelOfDetail < rhs.levelOfDetail;
    }
  };

  /// Level of details (zoom).
  int levelOfDetail;
  /// Tile x
  int tileX;
  /// Tile y
  int tileY;

  QuadKey() : QuadKey(0, 0, 0) {
  }

  QuadKey(int levelOfDetail, int tileX, int tileY) :
      levelOfDetail(levelOfDetail), tileX(tileX), tileY(tileY) {
  }

  bool operator==(const QuadKey &other) const {
    return levelOfDetail==other.levelOfDetail &&
        tileX==other.tileX && tileY==other.tileY;
  }
};

}
#endif // QUADKEY_HPP_DEFINED
