#include "builders/BuilderContext.hpp"
#include "builders/ExternalBuilder.hpp"
#include "builders/QuadKeyBuilder.hpp"

#include <set>

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::math;

namespace {
typedef std::unordered_map<std::string, QuadKeyBuilder::ElementBuilderFactory> BuilderFactoryMap;

/// Responsible for processing elements of quadkey in consistent way.
class BuilderElementVisitor : public ElementVisitor {
 public:
  BuilderElementVisitor(const BuilderContext &context, BuilderFactoryMap &builderFactoryMap) :
    context_(context), builderFactoryMap_(builderFactoryMap) { }

  void visitNode(const Node &node) override {
    visitElement(node);
  }

  void visitWay(const Way &way) override {
    visitElement(way);
  }

  void visitArea(const Area &area) override {
    visitElement(area);
  }

  void visitRelation(const Relation &relation) override {
    if (relation.tags.empty()) {
      // processing clipped element
      ids_.insert(relation.id);
      for (const auto &element : relation.elements)
        visitElement(*element);
    } else
      visitElement(relation);
  }

  void complete() {
    for (const auto &builder : builders_) {
      builder.second->complete();
    }
  }

 private:
  /// Calls appropriate visitor for given element
  void visitElement(const Element &element) {
    Style style = context_.styleProvider.forElement(element, context_.quadKey.levelOfDetail);

    if (canBuild(element, style)) {

      ids_.insert(element.id);

      for (const auto &name : style.getBuilders()) {
        element.accept(getBuilder(name));
      }
    }
  }

  bool canBuild(const Element &element, const Style &style) {
    // check do we know how to build it and prevent multiple building
    return !style.empty() && (element.id==0 || ids_.find(element.id)==ids_.end());
  }

  ElementBuilder &getBuilder(const std::string &name) {
    auto builderPair = builders_.find(name);
    if (builderPair!=builders_.end())
      return *builderPair->second;

    auto factory = builderFactoryMap_.find(name);
    builders_.emplace(name, factory==builderFactoryMap_.end()
      ? utymap::utils::make_unique<ExternalBuilder>(context_) // use external builder by default
      : factory->second(context_));

    auto &builder = *builders_[name];
    builder.prepare();

    return builder;
  }

  const BuilderContext &context_;
  BuilderFactoryMap &builderFactoryMap_;
  std::set<std::uint64_t> ids_;
  std::unordered_map<std::string, std::unique_ptr<ElementBuilder>> builders_;
};
}

class QuadKeyBuilder::QuadKeyBuilderImpl {
 public:
  QuadKeyBuilderImpl(GeoStore &geoStore, StringTable &stringTable) :
      geoStore_(geoStore), stringTable_(stringTable), builderFactory_() {}

  void registerElementVisitor(const std::string &name, ElementBuilderFactory factory) {
    builderFactory_[name] = factory;
  }

  void build(const QuadKey &quadKey,
             const StyleProvider &styleProvider,
             const ElevationProvider &eleProvider,
             const BuilderContext::MeshCallback &meshCallback,
             const BuilderContext::ElementCallback &elementCallback,
             const utymap::CancellationToken &cancelToken) {
    auto context = BuilderContext(quadKey, styleProvider, stringTable_, eleProvider,
      meshCallback, elementCallback, cancelToken);
    auto visitor = BuilderElementVisitor(context, builderFactory_);
    geoStore_.search(quadKey, styleProvider, visitor, cancelToken);
    visitor.complete();
  }

 private:
  GeoStore &geoStore_;
  StringTable &stringTable_;
  BuilderFactoryMap builderFactory_;
};

void QuadKeyBuilder::registerElementBuilder(const std::string &name, ElementBuilderFactory factory) {
  pimpl_->registerElementVisitor(name, factory);
}

void QuadKeyBuilder::build(const QuadKey &quadKey,
                           const StyleProvider &styleProvider,
                           const ElevationProvider &eleProvider,
                           const BuilderContext::MeshCallback &meshCallback,
                           const BuilderContext::ElementCallback &elementCallback,
                           const utymap::CancellationToken &cancelToken) {
  pimpl_->build(quadKey, styleProvider, eleProvider, meshCallback, elementCallback, cancelToken);
}

QuadKeyBuilder::QuadKeyBuilder(GeoStore &geoStore, StringTable &stringTable) :
    pimpl_(utymap::utils::make_unique<QuadKeyBuilderImpl>(geoStore, stringTable)) {}

QuadKeyBuilder::~QuadKeyBuilder() {}
