#ifndef LSYS_PARSER_HPP_DEFINED
#define LSYS_PARSER_HPP_DEFINED

#include "LSystem.hpp"

#include <string>
#include <istream>

namespace utymap {
namespace lsys {

/// Parses lsystem from internal lsys format.
/// lsys format is context-free stochastic L-system.
class LSystemParser final {
 public:
  /// Parses lsystem from string.
  LSystem parse(const std::string &content) const;

  /// Parses lsystem from input stream (e.g. file).
  LSystem parse(std::istream &istream) const;
};

}
}

#endif // LSYS_PARSER_HPP_DEFINED