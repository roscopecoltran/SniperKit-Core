#ifndef EXCEPTIONS_HPP_DEFINED
#define EXCEPTIONS_HPP_DEFINED

#include <stdexcept>
#include <string>

namespace utymap {

/// Thrown when exception is occured due to mapcss stylsheet processing.
class MapCssException final : public std::invalid_argument {
 public:
  explicit MapCssException(const std::string &message) :
      invalid_argument(message.c_str()) {
  }
};

}
#endif // EXCEPTIONS_HPP_DEFINED
