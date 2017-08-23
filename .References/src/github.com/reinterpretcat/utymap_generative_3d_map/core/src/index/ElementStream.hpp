#ifndef INDEX_ELEMENTSTREAM_HPP_DEFINED
#define INDEX_ELEMENTSTREAM_HPP_DEFINED

#include "entities/Element.hpp"

#include <memory>
#include <iostream>

namespace utymap {
namespace index {

class ElementStream final {
 public:
  /// Reads element with given id from input stream.
  static std::unique_ptr<utymap::entities::Element> read(std::istream &stream, std::uint64_t id);

  /// Writes element to output stream.
  static void write(std::ostream &stream, const utymap::entities::Element &element);
};

}
}

#endif // INDEX_ELEMENTSTREAM_HPP_DEFINED
