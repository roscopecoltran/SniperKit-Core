#ifndef INDEX_MESHSTREAM_HPP_DEFINED
#define INDEX_MESHSTREAM_HPP_DEFINED

#include "math/Mesh.hpp"
#include <iostream>

namespace utymap {
namespace index {

class MeshStream final {
 public:
  /// Reads mesh from input stream.
  static utymap::math::Mesh read(std::istream &stream);

  /// Writes mesh to output stream.
  static void write(std::ostream &stream, const utymap::math::Mesh &mesh);
};

}
}

#endif // INDEX_MESHSTREAM_HPP_DEFINED
