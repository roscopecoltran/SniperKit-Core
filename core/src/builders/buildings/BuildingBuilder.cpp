#include "GeoCoordinate.hpp"
#include "builders/MeshContext.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "builders/buildings/BuildingBuilder.hpp"
#include "builders/buildings/facades/CylinderFacadeBuilder.hpp"
#include "builders/buildings/facades/FlatFacadeBuilder.hpp"
#include "builders/buildings/facades/SphereFacadeBuilder.hpp"
#include "builders/buildings/roofs/DomeRoofBuilder.hpp"
#include "builders/buildings/roofs/FlatRoofBuilder.hpp"
#include "builders/buildings/roofs/PyramidalRoofBuilder.hpp"
#include "builders/buildings/roofs/MansardRoofBuilder.hpp"
#include "builders/buildings/roofs/SkillionRoofBuilder.hpp"
#include "builders/buildings/roofs/RoundRoofBuilder.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::index;
using namespace utymap::utils;

namespace {
const std::string MeshNamePrefix = "building:";

const std::string RoofPrefix = "roof-";
const std::string RoofTypeKey = RoofPrefix + StyleConsts::TypeKey();
const std::string RoofHeightKey = RoofPrefix + StyleConsts::HeightKey();
const std::string RoofGradientKey = RoofPrefix + StyleConsts::GradientKey();
const std::string RoofTextureIndexKey = RoofPrefix + StyleConsts::TextureIndexKey();
const std::string RoofTextureTypeKey = RoofPrefix + StyleConsts::TextureTypeKey();
const std::string RoofTextureScaleKey = RoofPrefix + StyleConsts::TextureScaleKey();
const std::string RoofDirectionKey = RoofPrefix + StyleConsts::DirectionKey();

const std::string FacadePrefix = "facade-";
const std::string FacadeTypeKey = FacadePrefix + StyleConsts::TypeKey();
const std::string FacadeGradientKey = FacadePrefix + StyleConsts::GradientKey();
const std::string FacadeTextureIndexKey = FacadePrefix + StyleConsts::TextureIndexKey();
const std::string FacadeTextureTypeKey = FacadePrefix + StyleConsts::TextureTypeKey();
const std::string FacadeTextureScaleKey = FacadePrefix + StyleConsts::TextureScaleKey();

/// Defines roof builder which does nothing.
class EmptyRoofBuilder : public RoofBuilder {
 public:
  EmptyRoofBuilder(const BuilderContext &bc, MeshContext &mc)
      : RoofBuilder(bc, mc) {}
  void build(Polygon &) override {}
};

typedef std::function<std::unique_ptr<RoofBuilder>(const BuilderContext &, MeshContext &)> RoofBuilderFactory;
std::unordered_map<std::string, RoofBuilderFactory> RoofBuilderFactoryMap =
    {
        {
            "none",
            [](const BuilderContext &builderContext, MeshContext &meshContext) {
              return utymap::utils::make_unique<EmptyRoofBuilder>(builderContext, meshContext);
            }
        },
        {
            "flat",
            [](const BuilderContext &builderContext, MeshContext &meshContext) {
              return utymap::utils::make_unique<FlatRoofBuilder>(builderContext, meshContext);
            }
        },
        {
            "dome",
            [](const BuilderContext &builderContext, MeshContext &meshContext) {
              return utymap::utils::make_unique<DomeRoofBuilder>(builderContext, meshContext);
            }
        },
        {
            "pyramidal",
            [](const BuilderContext &builderContext, MeshContext &meshContext) {
              return utymap::utils::make_unique<PyramidalRoofBuilder>(builderContext, meshContext);
            }
        },
        {
            "mansard",
            [](const BuilderContext &builderContext, MeshContext &meshContext) {
              return utymap::utils::make_unique<MansardRoofBuilder>(builderContext, meshContext);
            }
        },
        {
            "skillion",
            [](const BuilderContext &builderContext, MeshContext &meshContext) {
              return utymap::utils::make_unique<SkillionRoofBuilder>(builderContext, meshContext);
            }
        },
        {
            "round",
            [](const BuilderContext &builderContext, MeshContext &meshContext) {
              return utymap::utils::make_unique<RoundRoofBuilder>(builderContext, meshContext);
            }
        }
    };

typedef std::function<std::unique_ptr<FacadeBuilder>(const BuilderContext &, MeshContext &)> FacadeBuilderFactory;
std::unordered_map<std::string, FacadeBuilderFactory> FacadeBuilderFactoryMap =
    {
        {
            "flat",
            [](const BuilderContext &builderContext, MeshContext &meshContext) {
              return utymap::utils::make_unique<FlatFacadeBuilder>(builderContext, meshContext);
            }
        },
        {
            "cylinder",
            [](const BuilderContext &builderContext, MeshContext &meshContext) {
              return utymap::utils::make_unique<CylinderFacadeBuilder>(builderContext, meshContext);
            }
        },
        {
            "sphere",
            [](const BuilderContext &builderContext, MeshContext &meshContext) {
              return utymap::utils::make_unique<SphereFacadeBuilder>(builderContext, meshContext);
            }
        }
    };

/// Creates points for polygon
std::vector<Vector2> toPoints(const std::vector<GeoCoordinate> &coordinates) {
  std::vector<Vector2> points;
  points.reserve(coordinates.size());
  for (const auto &coordinate : coordinates) {
    points.push_back(Vector2(coordinate.longitude, coordinate.latitude));
  }

  return std::move(points);
}

/// NOTE this method exists due to special processing of all buildings parts of the
/// one relation. In general, we want to have ability to render complex building
/// as one game object. If area/relation is already part of relation then we
/// should avoid processing it as independent element. We cannot just delete
/// element from store as it might be a part of other relation.
inline bool shouldBeIgnored(const Element &element) {
  return hasIgnore(element.tags);
}

/// Responsible for processing multipolygon relation.
class MultiPolygonVisitor final : public ElementVisitor {
 public:

  explicit MultiPolygonVisitor(Polygon &polygon) :
      polygon_(polygon) {
  }

  void visitNode(const Node &node) override { fail(node); }

  void visitWay(const Way &way) override { fail(way); }

  virtual void visitRelation(const Relation &relation) override {
    for (const auto &element : relation.elements)
      element->accept(*this);
  }

  void visitArea(const Area &area) override {
    if (!utymap::utils::isClockwise(area.coordinates))
      polygon_.addContour(toPoints(area.coordinates));
    else
      polygon_.addHole(toPoints(area.coordinates));
  }

 private:
  static void fail(const utymap::entities::Element &element) {
    throw std::domain_error("Unexpected element in multipolygon: " + utymap::utils::toString(element.id));
  }
  Polygon &polygon_;
};
}

namespace utymap {
namespace builders {

class BuildingBuilder::BuildingBuilderImpl : public ElementBuilder {
 public:
  explicit BuildingBuilderImpl(const utymap::builders::BuilderContext &context) :
      ElementBuilder(context),
      id_(0) {
  }

  void visitNode(const Node &) override {}

  void visitWay(const Way &) override {}

  void visitArea(const Area &area) override {
    Style style = context_.styleProvider.forElement(area, context_.quadKey.levelOfDetail);

    // NOTE this might happen if relation contains not a building
    if (!isBuilding(style))
      return;

    bool justCreated = ensureContext(area);
    polygon_->addContour(toPoints(area.coordinates));
    build(area, style);

    completeIfNecessary(justCreated);
  }

  void visitRelation(const Relation &relation) override {
    if (relation.elements.empty())
      return;

    bool justCreated = ensureContext(relation);

    Style style = context_.styleProvider.forElement(relation, context_.quadKey.levelOfDetail);

    if (isMultipolygon(style) && isBuilding(style)) {
      MultiPolygonVisitor visitor(*polygon_);

      for (const auto &element : relation.elements)
        element->accept(visitor);

      build(relation, style);
    } else {
      for (const auto &element : relation.elements)
        element->accept(*this);
    }

    completeIfNecessary(justCreated);
  }

 private:

  bool ensureContext(const Element &element) {
    if (polygon_==nullptr)
      polygon_ = utymap::utils::make_unique<Polygon>(1, 0);

    if (mesh_==nullptr) {
      mesh_ = utymap::utils::make_unique<Mesh>(utymap::utils::getMeshName(MeshNamePrefix, element));
      id_ = element.id;
      return true;
    }

    return false;
  }

  void completeIfNecessary(bool justCreated) {
    if (justCreated) {
      context_.meshCallback(*mesh_);
      mesh_.reset();
    }
  }

  static bool isBuilding(const Style &style) {
    return style.getString("building")=="true";
  }

  static bool isMultipolygon(const Style &style) {
    return style.getString("multipolygon")=="true";
  }

  void build(const Element &element, const Style &style) {
    auto geoCoordinate = GeoCoordinate(polygon_->points[1], polygon_->points[0]);

    double height = style.getValue(StyleConsts::HeightKey());
    // NOTE do not allow height to be zero. This might happen due to the issues in input osm data.
    if (height==0)
      height = 10;

    double minHeight = style.getValue(StyleConsts::MinHeightKey());

    double elevation = context_.eleProvider.getElevation(context_.quadKey, geoCoordinate) + minHeight;

    height -= minHeight;

    attachRoof(*mesh_, style, elevation, height);

    // NOTE so far, attach floors only for buildings with minHeight
    if (minHeight > 0)
      attachFloors(*mesh_, style, elevation, height);

    attachFacade(*mesh_, style, elevation, height);

    polygon_.reset();
  }

  void attachRoof(Mesh &mesh, const Style &style, double elevation, double height) const {
    MeshContext roofMeshContext = MeshContext::create(mesh,
                                                      style,
                                                      context_.styleProvider,
                                                      RoofGradientKey,
                                                      RoofTextureIndexKey,
                                                      RoofTextureTypeKey,
                                                      RoofTextureScaleKey,
                                                      id_);

    auto roofType = roofMeshContext.style.getString(RoofTypeKey);
    double roofHeight = roofMeshContext.style.getValue(RoofHeightKey);
    auto direction = roofMeshContext.style.getString(RoofDirectionKey);

    auto roofBuilder = RoofBuilderFactoryMap.find(roofType)->second(context_, roofMeshContext);
    roofBuilder->setHeight(roofHeight);
    roofBuilder->setMinHeight(elevation + height);
    roofBuilder->setColorNoiseFreq(0);
    roofBuilder->setDirection(direction);
    roofBuilder->build(*polygon_);

    context_.meshBuilder.writeTextureMappingInfo(mesh, roofMeshContext.appearanceOptions);
  }

  void attachFloors(Mesh &mesh, const Style &style, double elevation, double height) const {
    MeshContext floorMeshContext = MeshContext::create(mesh,
                                                       style,
                                                       context_.styleProvider,
                                                       RoofGradientKey,
                                                       RoofTextureIndexKey,
                                                       RoofTextureTypeKey,
                                                       RoofTextureScaleKey,
                                                       id_);

    FlatRoofBuilder floorBuilder(context_, floorMeshContext);
    floorBuilder.setMinHeight(elevation);
    floorBuilder.setColorNoiseFreq(0);
    floorBuilder.flipSide();
    floorBuilder.build(*polygon_);

    context_.meshBuilder.writeTextureMappingInfo(mesh, floorMeshContext.appearanceOptions);
  }

  void attachFacade(Mesh &mesh, const Style &style, double elevation, double height) const {
    MeshContext facadeMeshContext = MeshContext::create(mesh,
                                                        style,
                                                        context_.styleProvider,
                                                        FacadeGradientKey,
                                                        FacadeTextureIndexKey,
                                                        FacadeTextureTypeKey,
                                                        FacadeTextureScaleKey,
                                                        id_);

    auto facadeType = facadeMeshContext.style.getString(FacadeTypeKey);
    auto facadeBuilder = FacadeBuilderFactoryMap.find(facadeType)->second(context_, facadeMeshContext);

    facadeBuilder->setHeight(height);
    facadeBuilder->setMinHeight(elevation);
    facadeBuilder->setColorNoiseFreq(0);
    facadeBuilder->build(*polygon_);

    context_.meshBuilder.writeTextureMappingInfo(mesh, facadeMeshContext.appearanceOptions);
  }

  std::unique_ptr<Polygon> polygon_;
  std::unique_ptr<Mesh> mesh_;
  std::uint64_t id_;
};

BuildingBuilder::BuildingBuilder(const BuilderContext &context)
    : ElementBuilder(context), pimpl_(utymap::utils::make_unique<BuildingBuilderImpl>(context)) {
}

BuildingBuilder::~BuildingBuilder() {}

void BuildingBuilder::visitArea(const Area &area) {
  if (!shouldBeIgnored(area))
    area.accept(*pimpl_);
}

void BuildingBuilder::visitRelation(const Relation &relation) {
  if (!shouldBeIgnored(relation))
    relation.accept(*pimpl_);
}

}
}
