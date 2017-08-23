#ifndef BUILDERS_EXTERNALBUILDER_HPP_DEFINED
#define BUILDERS_EXTERNALBUILDER_HPP_DEFINED

#include "builders/ElementBuilder.hpp"
#include "builders/BuilderContext.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"

namespace utymap {
namespace builders {

/// Provides the way to delegate building logic to external code.
class ExternalBuilder final : public utymap::builders::ElementBuilder {
 public:
  explicit ExternalBuilder(const utymap::builders::BuilderContext &context) :
      utymap::builders::ElementBuilder(context) {
  }

  void visitNode(const utymap::entities::Node &node) override {
    context_.elementCallback(node);
  }

  void visitWay(const utymap::entities::Way &way) override {
    context_.elementCallback(way);
  }

  void visitArea(const utymap::entities::Area &area) override {
    context_.elementCallback(area);
  }

  void visitRelation(const utymap::entities::Relation &relation) override {
    context_.elementCallback(relation);
  }
};

}
}

#endif // BUILDERS_EXTERNALBUILDER_HPP_DEFINED
