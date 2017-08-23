#ifndef BUILDERS_MISC_BARRIER_BUILDER_HPP_DEFINED
#define BUILDERS_MISC_BARRIER_BUILDER_HPP_DEFINED

#include "builders/ElementBuilder.hpp"
#include "builders/MeshContext.hpp"

namespace utymap {
namespace builders {

/// Provides the way to build barrier.
class BarrierBuilder final : public ElementBuilder {

 public:
  explicit BarrierBuilder(const utymap::builders::BuilderContext &context) :
      ElementBuilder(context) {
  }

  void visitNode(const utymap::entities::Node &) override;

  void visitArea(const utymap::entities::Area &) override;

  void visitWay(const utymap::entities::Way &way) override;

  void visitRelation(const utymap::entities::Relation &) override;

 private:
  typedef std::vector<GeoCoordinate>::const_iterator Iterator;

  /// Builds barrier from element.
  template<typename T>
  void build(const T &element, Iterator begin, Iterator end);

  /// Builds barrier as wall.
  template<typename T>
  void buildWall(const T &element, Iterator begin, Iterator end, MeshContext &meshContext);

  /// Builds barrier as pillars.
  template<typename T>
  void buildPillar(const T &element, Iterator begin, Iterator end, MeshContext &meshContext);

  /// Set style if necessary.
  bool setStyle(const utymap::entities::Element &element);

  /// Reset style if necessary.
  void resetStyle(bool isSet);

  /// Holds style of the element.
  std::unique_ptr<utymap::mapcss::Style> style_;
};

}
}
#endif // BUILDERS_MISC_BARRIER_BUILDER_HPP_DEFINED
