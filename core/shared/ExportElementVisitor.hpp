#ifndef EXPORTELEMENTVISITOR_HPP_DEFINED
#define EXPORTELEMENTVISITOR_HPP_DEFINED

#include "Callbacks.hpp"
#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Area.hpp"
#include "entities/Way.hpp"
#include "entities/Relation.hpp"
#include "heightmap/ElevationProvider.hpp"
#include "mapcss/StyleProvider.hpp"

#include <string>
#include <vector>

/// Exports elements to external code using element callback.
struct ExportElementVisitor : public utymap::entities::ElementVisitor {
  using Tags = std::vector<utymap::formats::Tag>;
  using Coordinates = std::vector<utymap::GeoCoordinate>;

  ExportElementVisitor(int tag,
                       const utymap::QuadKey &quadKey,
                       utymap::index::StringTable &stringTable,
                       const utymap::mapcss::StyleProvider &styleProvider,
                       const utymap::heightmap::ElevationProvider &eleProvider,
                       OnElementLoaded *elementCallback) :
      tag_(tag), quadKey_(quadKey), stringTable_(stringTable), styleProvider_(styleProvider),
      eleProvider_(eleProvider), elementCallback_(elementCallback) {
  }

  void visitNode(const utymap::entities::Node &node) override {
    visitElement(node, Coordinates{node.coordinate});
  }

  void visitWay(const utymap::entities::Way &way) override {
    visitElement(way, way.coordinates);
  }

  void visitArea(const utymap::entities::Area &area) override {
    visitElement(area, area.coordinates);
  }

  void visitRelation(const utymap::entities::Relation &relation) override {
    // TODO not supported yet.
  }
 private:

  void visitElement(const utymap::entities::Element &element, const Coordinates &coordinates) {
    // convert tags
    std::vector<const char *> ctags;
    tagStrings_.reserve(element.tags.size()*2);
    ctags.reserve(element.tags.size()*2);
    for (std::size_t i = 0; i < element.tags.size(); ++i) {
      const utymap::entities::Tag &tag = element.tags[i];
      tagStrings_.push_back(stringTable_.getString(tag.key));
      tagStrings_.push_back(stringTable_.getString(tag.value));
      ctags.push_back(tagStrings_[tagStrings_.size() - 2].c_str());
      ctags.push_back(tagStrings_[tagStrings_.size() - 1].c_str());
    }
    // convert geometry
    vertices_.reserve(coordinates.size()*3);
    for (std::size_t i = 0; i < coordinates.size(); ++i) {
      const utymap::GeoCoordinate coordinate = coordinates[i];
      vertices_.push_back(coordinate.longitude);
      vertices_.push_back(coordinate.latitude);
      vertices_.push_back(eleProvider_.getElevation(quadKey_, coordinate));
    }
    // convert style
    utymap::mapcss::Style style = styleProvider_.forElement(element, quadKey_.levelOfDetail);
    std::vector<const char *> cstyles;
    auto declarations = style.declarations();
    styleStrings_.reserve(declarations.size()*2);
    cstyles.reserve(declarations.size());
    for (const auto &declaration : declarations) {
      styleStrings_.push_back(stringTable_.getString(declaration->key()));
      styleStrings_.push_back(declaration->value());
      cstyles.push_back(styleStrings_[styleStrings_.size() - 2].c_str());
      cstyles.push_back(styleStrings_[styleStrings_.size() - 1].c_str());
    }

    elementCallback_(tag_, element.id,
                     ctags.data(), static_cast<int>(ctags.size()),
                     vertices_.data(), static_cast<int>(vertices_.size()),
                     cstyles.data(), static_cast<int>(cstyles.size()));

    // NOTE clear vectors after raw array data is consumed by external code
    vertices_.clear();
    tagStrings_.clear();
    styleStrings_.clear();
  }

  const int tag_;
  const utymap::QuadKey &quadKey_;
  utymap::index::StringTable &stringTable_;
  const utymap::mapcss::StyleProvider &styleProvider_;
  const utymap::heightmap::ElevationProvider &eleProvider_;
  OnElementLoaded *elementCallback_;
  std::vector<double> vertices_;
  std::vector<std::string> tagStrings_;   // holds temporary tag strings
  std::vector<std::string> styleStrings_; // holds temporary style strings
};

#endif // EXPORTELEMENTVISITOR_HPP_DEFINED
