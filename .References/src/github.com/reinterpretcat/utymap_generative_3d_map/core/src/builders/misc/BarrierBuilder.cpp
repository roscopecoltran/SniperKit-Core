#include "builders/generators/CylinderGenerator.hpp"
#include "builders/generators/WallGenerator.hpp"
#include "builders/misc/BarrierBuilder.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "utils/GeometryUtils.hpp"
#include "utils/MeshUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::utils;

namespace {
const double Scale = 1E7;
const std::string MeshNamePrefix = "barrier:";
const std::string PillarType = "pillar";
}

void BarrierBuilder::visitNode(const Node &node) {
  std::vector<GeoCoordinate> geometry = {node.coordinate};
  build(node, geometry.begin(), geometry.end());
}

void BarrierBuilder::visitWay(const Way &way) {
  build(way, way.coordinates.begin(), way.coordinates.end());
}

void BarrierBuilder::visitArea(const Area &area) {
  build(area, area.coordinates.begin(), area.coordinates.end());
}

void BarrierBuilder::visitRelation(const Relation &relation) {
  bool isSet = setStyle(relation);
  for (const auto &element : relation.elements)
    element->accept(*this);

  resetStyle(isSet);
}

bool BarrierBuilder::setStyle(const utymap::entities::Element &element) {
  if (style_==nullptr) {
    style_ =
        utymap::utils::make_unique<Style>(context_.styleProvider.forElement(element, context_.quadKey.levelOfDetail));
    return true;
  }

  return false;
}

void BarrierBuilder::resetStyle(bool isSet) {
  if (isSet)
    style_.reset();
}

template<typename T>
void BarrierBuilder::build(const T &element, Iterator begin, Iterator end) {
  bool isSet = setStyle(element);

  Mesh mesh(utymap::utils::getMeshName(MeshNamePrefix, element));
  MeshContext meshContext = MeshContext::create(mesh, *style_, context_.styleProvider, element.id);

  if (style_->getString(StyleConsts::TypeKey())==PillarType)
    buildPillar(element, begin, end, meshContext);
  else
    buildWall(element, begin, end, meshContext);

  resetStyle(isSet);
}

template<typename T>
void BarrierBuilder::buildWall(const T &element, Iterator begin, Iterator end, MeshContext &meshContext) {
  double width = meshContext.style.getValue(StyleConsts::WidthKey(), context_.boundingBox);
  WallGenerator generator(context_, meshContext);
  generator
      .setGeometry(begin, end)
      .setWidth(width)
      .setHeight(meshContext.style.getValue(StyleConsts::HeightKey()))
      .setLength(meshContext.style.getValue(StyleConsts::LengthKey()))
      .setGap(meshContext.style.getValue(StyleConsts::GapKey()))
      .generate();
  context_.meshCallback(meshContext.mesh);
}

template<typename T>
void BarrierBuilder::buildPillar(const T &element, Iterator begin, Iterator end, MeshContext &meshContext) {
  auto size = std::distance(begin, end);
  if (size==0)
    return;

  auto radius = utymap::utils::getSize(context_.boundingBox, meshContext.style, StyleConsts::RadiusKey());
  auto height = meshContext.style.getValue(StyleConsts::HeightKey());

  Mesh mesh(utymap::utils::getMeshName(MeshNamePrefix, element));
  CylinderGenerator generator(context_, meshContext);
  generator
      .setCenter(Vector3(begin->longitude,
                         context_.eleProvider.getElevation(context_.quadKey, *begin),
                         begin->latitude))
      .setSize(Vector3(radius.x, height, radius.z))
      .setMaxSegmentHeight(5)
      .setRadialSegments(7)
      .setVertexNoiseFreq(0)
      .generate();

  context_.meshBuilder.writeTextureMappingInfo(meshContext.mesh, meshContext.appearanceOptions);

  // NOTE single coordinate, no need to replicate
  if (size==1) {
    context_.meshCallback(meshContext.mesh);
    return;
  }

  double treeStepInMeters = meshContext.style.getValue(StyleConsts::StepKey());
  for (std::size_t i = 0; i < static_cast<std::size_t>(size - 1); ++i) {
    const auto p0 = (begin + i);
    const auto p1 = (begin + i + 1);
    utymap::utils::copyMeshAlong(context_.quadKey, *begin, *p0, *p1,
                                 meshContext.mesh, mesh, treeStepInMeters, context_.eleProvider);
  }
  context_.meshCallback(mesh);
}
