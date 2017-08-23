#ifndef BUILDERS_POI_TREEBUILDER_HPP_DEFINED
#define BUILDERS_POI_TREEBUILDER_HPP_DEFINED

#include "builders/ElementBuilder.hpp"
#include "entities/Area.hpp"

namespace utymap {
namespace builders {

/// Builds single tree.
class TreeBuilder final : public utymap::builders::ElementBuilder {
 public:
  explicit TreeBuilder(const utymap::builders::BuilderContext &context) :
      utymap::builders::ElementBuilder(context) {
  }

  void visitNode(const utymap::entities::Node &node) override;

  void visitWay(const utymap::entities::Way &way) override;

  void visitArea(const utymap::entities::Area &area) override {}

  void visitRelation(const utymap::entities::Relation &relation) override;
};

}
}

#endif // BUILDERS_POI_TREEBUILDER_HPP_DEFINED
