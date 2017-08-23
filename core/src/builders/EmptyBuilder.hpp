#ifndef BUILDERS_EMPTYBUILDER_HPP_DEFINED
#define BUILDERS_EMPTYBUILDER_HPP_DEFINED

#include "builders/ElementBuilder.hpp"
#include "builders/BuilderContext.hpp"

namespace utymap {
namespace builders {

/// Empty builder which does nothing.
class EmptyBuilder final : public ElementBuilder {
 public:
  explicit EmptyBuilder(const BuilderContext &context) : ElementBuilder(context) {}

  void visitNode(const utymap::entities::Node &) override {}

  void visitWay(const utymap::entities::Way &) override {}

  void visitArea(const utymap::entities::Area &) override {}

  void visitRelation(const utymap::entities::Relation &) override {}
};

}
}

#endif // BUILDERS_EMPTYBUILDER_HPP_DEFINED
