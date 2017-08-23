#ifndef ENTITIES_ELEMENTVISITOR_HPP_DEFINED
#define ENTITIES_ELEMENTVISITOR_HPP_DEFINED

namespace utymap {
namespace entities {

struct Node;
struct Way;
struct Area;
struct Relation;

/// A base class for visiting entities - elements.
class ElementVisitor {
 public:
  /// Visits node.
  virtual void visitNode(const Node &) = 0;

  /// Visits way.
  virtual void visitWay(const Way &) = 0;

  /// Visits area.
  virtual void visitArea(const Area &) = 0;

  /// Visits relation.
  virtual void visitRelation(const Relation &) = 0;

  virtual ~ElementVisitor() = default;
};

}
}

#endif // ENTITIES_ELEMENTVISITOR_HPP_DEFINED
