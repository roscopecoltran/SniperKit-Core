#ifndef MAPCSS_MAPCSSPARSER_HPP_DEFINED
#define MAPCSS_MAPCSSPARSER_HPP_DEFINED

#include "mapcss/StyleSheet.hpp"

#include <istream>
#include <string>

namespace utymap {
namespace mapcss {

/// Provides the way to parse mapcss data into stylesheets object graph.
class MapCssParser final {
 public:

  /// Directory parameter specifies root path for import lookup.
  explicit MapCssParser(const std::string &directory = "");

  /// Parses stylesheet from string.
  StyleSheet parse(const std::string &content) const;

  /// Parses stylesheet from input stream (e.g. file).
  StyleSheet parse(std::istream &istream) const;

 private:
  std::string directory_;
};

}
}
#endif // MAPCSS_MAPCSSPARSER_HPP_DEFINED
