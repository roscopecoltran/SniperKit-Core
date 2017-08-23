#include "builders/terrain/SurfaceGenerator.hpp"

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::utils;

using namespace std::placeholders;

namespace {
const std::string TerrainMeshName = "terrain_surface";
const int Level = 0;

const std::unordered_map<std::string, TerraExtras::ExtrasFunc> ExtrasFuncs =
    {
        {"forest", std::bind(&TerraExtras::addForest, _1, _2)},
        {"water", std::bind(&TerraExtras::addWater, _1, _2)},
    };
};

SurfaceGenerator::SurfaceGenerator(const BuilderContext &context, const Style &style, const Path &tileRect) :
    TerraGenerator(context, style, tileRect, TerrainMeshName) {
}

void SurfaceGenerator::onNewRegion(const std::string &type,
                                   const utymap::entities::Element &element,
                                   const Style &style,
                                   const std::shared_ptr<Region> &region) {
}

/// NOTE Original layer collection is cleared after this function executed.
void SurfaceGenerator::generateFrom(const std::vector<Layer> &layers) {
  buildForeground(layers);

  buildBackground();

  context_.meshCallback(mesh_);
}

void SurfaceGenerator::buildForeground(const std::vector<Layer> &layers) {
  for (const auto &layer : layers)
    buildLayer(layer);
}

void SurfaceGenerator::buildBackground() {
  Paths background;
  backgroundClipper_.AddPath(tileRect_, ptSubject, true);
  backgroundClipper_.Execute(ctDifference, background, pftNonZero, pftNonZero);
  backgroundClipper_.Clear();

  if (!background.empty())
    TerraGenerator::addGeometry(Level, background, RegionContext::create(context_, style_, ""),
                                [](const Path &path) {});
}

void SurfaceGenerator::buildLayer(const Layer &layer) {
  for (const auto &region : layer.regions) {
    // TODO revise condition once level processing is implemented
    // if (region->level == 0)
    buildRegion(*region);
  }
}

void SurfaceGenerator::buildRegion(const Region &region) {
  Paths solution;
  foregroundClipper_.AddPaths(region.geometry, ptSubject, true);
  foregroundClipper_.Execute(ctDifference, solution, pftNonZero, pftNonZero);
  foregroundClipper_.moveSubjectToClip();

  TerraGenerator::addGeometry(Level, solution, *region.context, [&](const Path &path) {
    backgroundClipper_.AddPath(path, ptClip, true);
  });
}

void SurfaceGenerator::addGeometry(int level, Polygon &polygon, const RegionContext &regionContext) {
  std::string meshName = regionContext.style.getString(regionContext.prefix + StyleConsts::MeshNameKey());
  if (!meshName.empty()) {
    Mesh polygonMesh(meshName);
    TerraExtras::Context extrasContext(polygonMesh, regionContext.style);
    context_.meshBuilder.addPolygon(polygonMesh, polygon,
                                    regionContext.geometryOptions, regionContext.appearanceOptions);
    context_.meshBuilder.writeTextureMappingInfo(polygonMesh, regionContext.appearanceOptions);

    addExtrasIfNecessary(polygonMesh, extrasContext, regionContext);
    context_.meshCallback(polygonMesh);
  } else {
    TerraExtras::Context extrasContext(mesh_, regionContext.style);
    context_.meshBuilder.addPolygon(mesh_, polygon,
                                    regionContext.geometryOptions, regionContext.appearanceOptions);
    context_.meshBuilder.writeTextureMappingInfo(mesh_, regionContext.appearanceOptions);

    addExtrasIfNecessary(mesh_, extrasContext, regionContext);
  }
}

void SurfaceGenerator::addExtrasIfNecessary(Mesh &mesh,
                                            TerraExtras::Context &extrasContext,
                                            const RegionContext &regionContext) const {
  auto meshExtras = regionContext.style.getString(regionContext.prefix + StyleConsts::MeshExtrasKey());
  if (meshExtras.empty())
    return;

  ExtrasFuncs.at(meshExtras)(context_, extrasContext);
}
