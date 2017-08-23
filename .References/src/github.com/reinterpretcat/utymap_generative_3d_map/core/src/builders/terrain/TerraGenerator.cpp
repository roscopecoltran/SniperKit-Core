#include "builders/terrain/TerraGenerator.hpp"

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::math;

namespace {
/// Tolerance for meshing
const double AreaTolerance = 1000;
/// Coordinate scale.
const double Scale = 1E7;
}

TerraGenerator::TerraGenerator(const utymap::builders::BuilderContext &context,
                               const utymap::mapcss::Style &style,
                               const ClipperLib::Path &tileRect,
                               const std::string &meshName) :
    context_(context), style_(style),
    tileRect_(tileRect),
    mesh_(meshName),
    rect_(context.boundingBox.minPoint.longitude,
          context.boundingBox.minPoint.latitude,
          context.boundingBox.maxPoint.longitude,
          context.boundingBox.maxPoint.latitude) {
  // find relative size which fits for all quadkeys on given LOD
  auto relativeQuadKey =
      utymap::utils::GeoUtils::GeoCoordinateToQuadKey(GeoCoordinate(0, 0), context.quadKey.levelOfDetail);
  auto relativeBbox = utymap::utils::GeoUtils::quadKeyToBoundingBox(relativeQuadKey);
  auto size = style_.getValue(StyleConsts::GridCellSize(), relativeBbox);

  splitter_.setParams(Scale, size);
}

void TerraGenerator::addGeometry(int level,
                                 Paths &geometry,
                                 const RegionContext &regionContext,
                                 const std::function<void(const Path &)> &geometryVisitor) {
  ClipperLib::SimplifyPolygons(geometry);
  ClipperLib::CleanPolygons(geometry);

  bool hasHeightOffset = std::abs(regionContext.geometryOptions.heightOffset) > 0;
  // calculate approximate size of overall points
  double size = 0;
  for (std::size_t i = 0; i < geometry.size(); ++i)
    size += geometry[i].size()*1.5;

  Polygon polygon(static_cast<std::size_t>(size));
  for (const Path &path : geometry) {
    double area = ClipperLib::Area(path);
    bool isHole = area < 0;
    if (std::abs(area) < AreaTolerance)
      continue;

    geometryVisitor(path);

    auto points = restoreGeometry(path);
    if (isHole)
      polygon.addHole(points);
    else
      polygon.addContour(points);

    if (hasHeightOffset)
      buildHeightOffset(points, regionContext);
  }

  if (!polygon.points.empty())
    addGeometry(level, polygon, regionContext);
}

void TerraGenerator::buildHeightOffset(const std::vector<Vector2> &points, const RegionContext &regionContext) {
  // do not use elevation noise for height offset.
  auto newGeometryOptions = regionContext.geometryOptions;
  newGeometryOptions.eleNoiseFreq = 0;

  for (std::size_t i = 0; i < points.size(); ++i) {
    const auto &p1 = points[i];
    const auto &p2 = points[i==(points.size() - 1) ? 0 : i + 1];

    // check whether two points are on cell rect
    if (isOnBorder(p1) && isOnBorder(p2)) continue;

    context_.meshBuilder.addPlane(mesh_, p1, p2, newGeometryOptions, regionContext.appearanceOptions);
  }
}

std::vector<Vector2> TerraGenerator::restoreGeometry(const Path &geometry) const {
  auto lastItemIndex = geometry.size() - 1;
  std::vector<utymap::math::Vector2> points;
  points.reserve(geometry.size());
  for (std::size_t i = 0; i <= lastItemIndex; i++)
    splitter_.split(geometry[i], geometry[i==lastItemIndex ? 0 : i + 1], points);

  return std::move(points);
}
