#ifndef BUILDERS_TERRAIN_TERRABUILDER_HPP_DEFINED
#define BUILDERS_TERRAIN_TERRABUILDER_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/ElementBuilder.hpp"

#include <memory>

namespace utymap {
namespace builders {

/// Provides the way to build terrain mesh.
class TerraBuilder final : public utymap::builders::ElementBuilder {
 public:

  explicit TerraBuilder(const utymap::builders::BuilderContext &);

  virtual ~TerraBuilder();

  void visitNode(const utymap::entities::Node &) override;

  void visitWay(const utymap::entities::Way &) override;

  void visitArea(const utymap::entities::Area &) override;

  void visitRelation(const utymap::entities::Relation &) override;

  void complete() override;

 private:
  class TerraBuilderImpl;
  std::unique_ptr<TerraBuilderImpl> pimpl_;
};

}
}

#endif // BUILDERS_TERRAIN_TERRABUILDER_HPP_DEFINED
