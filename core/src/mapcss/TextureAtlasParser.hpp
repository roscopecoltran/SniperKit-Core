#ifndef MAPCSS_TEXTUREATLASPARSER_HPP_DEFINED
#define MAPCSS_TEXTUREATLASPARSER_HPP_DEFINED

#include "mapcss/StyleSheet.hpp"

#include <istream>
#include <string>

namespace utymap {
namespace mapcss {

/// Provides the way to parse mapcss data into stylesheets object graph.
class TextureAtlasParser final {
 public:
  /// Parses texture atlas from string.
  TextureAtlas parse(const std::string &content) const;

  /// Parses texture atlas from input stream (e.g. file).
  TextureAtlas parse(std::istream &istream) const;
};

}
}
#endif // MAPCSS_TEXTUREATLASPARSER_HPP_DEFINED
