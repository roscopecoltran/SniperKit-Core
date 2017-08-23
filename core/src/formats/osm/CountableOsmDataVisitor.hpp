#ifndef TESTS_FORMATS_COUNTABLEOSMDATAVISITOR_HPP_DEFINED
#define TESTS_FORMATS_COUNTABLEOSMDATAVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "formats/FormatTypes.hpp"

#include <cstdint>

namespace utymap {
namespace formats {

struct CountableOsmDataVisitor : public utymap::entities::ElementVisitor {
  int bounds;
  int nodes;
  int ways;
  int areas;
  int relations;

  CountableOsmDataVisitor() :
      bounds(0), nodes(0), ways(0), areas(0), relations(0) {
  }

  void visitBounds(utymap::BoundingBox bbox) {
    bounds++;
  }

  void visitNode(uint64_t id, utymap::GeoCoordinate &coordinate, Tags &tags) {
    nodes++;
  }

  void visitWay(uint64_t id, std::vector<uint64_t> &nodeIds, Tags &tags) {
    ways++;
  }

  void visitRelation(uint64_t id, RelationMembers &members, Tags &tags) {
    relations++;
  }

  void visitNode(const utymap::entities::Node &) override {
    ++nodes;
  }

  void visitWay(const utymap::entities::Way &) override {
    ++ways;
  }

  void visitArea(const utymap::entities::Area &) override {
    ++areas;
  }

  void visitRelation(const utymap::entities::Relation &) override {
    ++relations;
  }

  void add(utymap::entities::Element &element) {
    element.accept(*this);
  }
};

}
}

#endif // TESTS_FORMATS_COUNTABLEOSMDATAVISITOR_HPP_DEFINED
