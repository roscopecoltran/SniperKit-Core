#ifndef BUILDERS_ELEMENTBUILDER_HPP_DEFINED
#define BUILDERS_ELEMENTBUILDER_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "entities/ElementVisitor.hpp"

namespace utymap {
namespace builders {

/// Provides the way to build specific meshes from map data for given quadkey.
class ElementBuilder : public utymap::entities::ElementVisitor {
 public:
  explicit ElementBuilder(const utymap::builders::BuilderContext &context) :
      context_(context) {}

  // Called before processing is started.
  virtual void prepare() {}

  /// Called when processing is finished.
  /// This happens when all objects for the corresponding quadkey are processed.
  virtual void complete() {}

 protected:
  const utymap::builders::BuilderContext &context_;
};

}
}
#endif // BUILDERS_ELEMENTBUILDER_HPP_DEFINED
