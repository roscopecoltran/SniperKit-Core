#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "index/InMemoryElementStore.hpp"

using namespace utymap;
using namespace utymap::index;
using namespace utymap::entities;
using namespace utymap::mapcss;

namespace {
typedef std::vector<std::shared_ptr<Element>> Elements;
typedef std::map<QuadKey, Elements, QuadKey::Comparator> ElementMap;

class ElementMapVisitor : public ElementVisitor {
 public:
  ElementMapVisitor(const QuadKey &quadKey, ElementMap &elementsMap) :
      quadKey_(quadKey), elementsMap_(elementsMap) {
  }

  void visitNode(const utymap::entities::Node &node) override {
    elementsMap_[quadKey_].push_back(std::make_shared<Node>(node));
  }

  void visitWay(const utymap::entities::Way &way) override {
    elementsMap_[quadKey_].push_back(std::make_shared<Way>(way));
  }

  void visitArea(const utymap::entities::Area &area) override {
    elementsMap_[quadKey_].push_back(std::make_shared<Area>(area));
  }

  void visitRelation(const utymap::entities::Relation &relation) override {
    elementsMap_[quadKey_].push_back(std::make_shared<Relation>(relation));
  }
 private:
  const QuadKey &quadKey_;
  ElementMap &elementsMap_;
};
}

class InMemoryElementStore::InMemoryElementStoreImpl {
 public:
  ElementMap elementsMap;

  ElementMap::const_iterator begin(const utymap::QuadKey &quadKey) const {
    return elementsMap.find(quadKey);
  }

  ElementMap::const_iterator end() const {
    return elementsMap.cend();
  }

  bool hasData(const utymap::QuadKey &quadKey) const {
    return elementsMap.find(quadKey)!=elementsMap.end();
  }
};

InMemoryElementStore::InMemoryElementStore(const StringTable &stringTable) :
    ElementStore(stringTable), pimpl_(utymap::utils::make_unique<InMemoryElementStoreImpl>()) {
}

InMemoryElementStore::~InMemoryElementStore() {
}

void InMemoryElementStore::storeImpl(const utymap::entities::Element &element, const QuadKey &quadKey) {
  ElementMapVisitor visitor(quadKey, pimpl_->elementsMap);
  element.accept(visitor);
}

bool InMemoryElementStore::hasData(const utymap::QuadKey &quadKey) const {
  return pimpl_->hasData(quadKey);
}

void InMemoryElementStore::search(const utymap::QuadKey &quadKey,
                                  utymap::entities::ElementVisitor &visitor,
                                  const utymap::CancellationToken &cancelToken) {
  auto it = pimpl_->begin(quadKey);

  // No elements for this quadkey
  if (it==pimpl_->end())
    return;

  for (const auto &element : it->second) {
    if (cancelToken.isCancelled())
      break;

    element->accept(visitor);
  }
}
