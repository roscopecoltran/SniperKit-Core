#ifndef BUILDERS_MISC_LAMPBUILDER_HPP_DEFINED
#define BUILDERS_MISC_LAMPBUILDER_HPP_DEFINED

#include "builders/ElementBuilder.hpp"

namespace utymap {
namespace builders {

/// Provides the way to build street lights.
class LampBuilder final : public ElementBuilder {
 public:
  explicit LampBuilder(const utymap::builders::BuilderContext &context) :
      ElementBuilder(context) {
  }

  void visitNode(const utymap::entities::Node &) override;

  void visitArea(const utymap::entities::Area &) override {}

  void visitWay(const utymap::entities::Way &way) override;

  void visitRelation(const utymap::entities::Relation &) override;
};

}
}
#endif // BUILDERS_MISC_LAMPBUILDER_HPP_DEFINED
