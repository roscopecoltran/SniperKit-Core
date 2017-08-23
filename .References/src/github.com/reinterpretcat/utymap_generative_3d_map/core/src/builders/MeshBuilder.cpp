#define REAL double
#define ANSI_DECLARATORS

#include "BoundingBox.hpp"
#include "MeshBuilder.hpp"
#include "triangle/triangle.h"
#include "utils/CoreUtils.hpp"
#include "utils/GeoUtils.hpp"
#include "utils/GradientUtils.hpp"

#include <mutex>

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::heightmap;
using namespace utymap::math;
using namespace utymap::utils;

namespace {
// NOTE triangle library is not thread safe.
std::mutex lock_;

/// Creates texture mapping function.
std::function<Vector2(double, double)> createMapFunc(const MeshBuilder::AppearanceOptions &appearanceOptions,
                                                     const BoundingBox &bbox) {
  if (appearanceOptions.textureRegion.isEmpty()) {
    return [](double x, double y) {
      return Vector2(0, 0);
    };
  }

  double geoWidth = bbox.width();
  double geoHeight = bbox.height();
  double geoX = bbox.minPoint.longitude;
  double geoY = bbox.minPoint.latitude;

  auto scale = appearanceOptions.textureScale;

  return [=](double x, double y) {
    double relX = (x - geoX)/geoWidth;
    double relY = (y - geoY)/geoHeight;
    return Vector2(relX*scale, relY*scale);
  };
}

void ensureMeshCapacity(Mesh &mesh, std::size_t pointCount, std::size_t triCount) {
  mesh.vertices.reserve(mesh.vertices.size() + pointCount*3/2);
  mesh.triangles.reserve(mesh.triangles.size() + triCount*3);
  mesh.colors.reserve(mesh.colors.size() + pointCount);
  mesh.uvs.reserve(mesh.uvs.size() + pointCount*2);
}

/// Fills mesh with all data needed to render object correctly outside core library.
void fillMesh(triangulateio *io, QuadKey quadKey, const BoundingBox &bbox, Mesh &mesh,
              const ElevationProvider &eleProvider,
              const MeshBuilder::GeometryOptions &geometryOptions,
              const MeshBuilder::AppearanceOptions &appearanceOptions) {
  int triStartIndex = static_cast<int>(mesh.vertices.size()/3);

  // prepare texture data
  const auto map = createMapFunc(appearanceOptions, bbox);

  ensureMeshCapacity(mesh, static_cast<std::size_t>(io->numberofpoints),
                     static_cast<std::size_t>(io->numberoftriangles));

  for (int i = 0; i < io->numberofpoints; i++) {
    // get coordinates
    double x = io->pointlist[i*2 + 0];
    double y = io->pointlist[i*2 + 1];

    double ele = geometryOptions.heightOffset +
        (geometryOptions.elevation > std::numeric_limits<double>::lowest()
         ? geometryOptions.elevation
         : eleProvider.getElevation(quadKey, y, x));

    // do no apply noise on boundaries
    if (io->pointmarkerlist!=nullptr && io->pointmarkerlist[i]!=1)
      ele += NoiseUtils::perlin2D(x, y, geometryOptions.eleNoiseFreq);

    // set vertices
    mesh.vertices.push_back(x);
    mesh.vertices.push_back(y);
    mesh.vertices.push_back(ele);

    // set colors
    int color = GradientUtils::getColor(appearanceOptions.gradient, x, y, appearanceOptions.colorNoiseFreq);
    mesh.colors.push_back(color);

    // set textures
    const auto uv = map(x, y);
    mesh.uvs.push_back(uv.x);
    mesh.uvs.push_back(uv.y);
  }

  // set triangles
  int first = geometryOptions.flipSide ? 2 : 1;
  int second = 0;
  int third = geometryOptions.flipSide ? 1 : 2;

  for (int i = 0; i < io->numberoftriangles; i++) {
    mesh.triangles.push_back(triStartIndex + io->trianglelist[i*io->numberofcorners + first]);
    mesh.triangles.push_back(triStartIndex + io->trianglelist[i*io->numberofcorners + second]);
    mesh.triangles.push_back(triStartIndex + io->trianglelist[i*io->numberofcorners + third]);
  }
}
}

MeshBuilder::MeshBuilder(const utymap::QuadKey &quadKey, const ElevationProvider &eleProvider) :
    quadKey_(quadKey),
    bbox_(GeoUtils::quadKeyToBoundingBox(quadKey)),
    eleProvider_(eleProvider) {
}

MeshBuilder::~MeshBuilder() {}

void MeshBuilder::addPolygon(Mesh &mesh,
                             Polygon &polygon,
                             const GeometryOptions &geometryOptions,
                             const AppearanceOptions &appearanceOptions) const {
  triangulateio in, mid;

  in.numberofpoints = static_cast<int>(polygon.points.size()/2);
  in.numberofholes = static_cast<int>(polygon.holes.size()/2);
  in.numberofpointattributes = 0;
  in.numberofregions = 0;
  in.numberofsegments = static_cast<int>(polygon.segments.size()/2);

  in.pointlist = polygon.points.data();
  in.holelist = polygon.holes.data();
  in.segmentlist = polygon.segments.data();
  in.segmentmarkerlist = nullptr;
  in.pointmarkerlist = nullptr;

  mid.pointlist = nullptr;
  mid.pointmarkerlist = nullptr;
  mid.trianglelist = nullptr;
  mid.segmentlist = nullptr;
  mid.segmentmarkerlist = nullptr;

  {
    std::lock_guard<std::mutex> lock(lock_);
    ::triangulate(const_cast<char *>("pzBQ"), &in, &mid, nullptr);
  }

  // do not refine mesh if area is not set.
  if (std::abs(geometryOptions.area) < std::numeric_limits<double>::epsilon()) {
    fillMesh(&mid, quadKey_, bbox_, mesh, eleProvider_, geometryOptions, appearanceOptions);
    mid.trianglearealist = nullptr;
  } else {

    mid.trianglearealist = static_cast<REAL *>(malloc(mid.numberoftriangles*sizeof(REAL)));
    for (int i = 0; i < mid.numberoftriangles; ++i) {
      mid.trianglearealist[i] = geometryOptions.area;
    }

    triangulateio out;
    out.pointlist = nullptr;
    out.pointattributelist = nullptr;
    out.trianglelist = nullptr;
    out.triangleattributelist = nullptr;
    out.pointmarkerlist = nullptr;

    std::string triOptions = "prazPQ";
    for (int i = 0; i < geometryOptions.segmentSplit; i++) {
      triOptions += "Y";
    }

    {
      std::lock_guard<std::mutex> lock(lock_);
      ::triangulate(const_cast<char *>(triOptions.c_str()), &mid, &out, nullptr);
    }

    fillMesh(&out, quadKey_, bbox_, mesh, eleProvider_, geometryOptions, appearanceOptions);

    free(out.pointlist);
    free(out.pointattributelist);
    free(out.trianglelist);
    free(out.triangleattributelist);
    free(out.pointmarkerlist);
  }

  free(in.pointmarkerlist);

  free(mid.pointlist);
  free(mid.pointmarkerlist);
  free(mid.trianglelist);
  free(mid.trianglearealist);
  free(mid.segmentlist);
  free(mid.segmentmarkerlist);
}

void MeshBuilder::addPlane(Mesh &mesh,
                           const Vector2 &p1,
                           const Vector2 &p2,
                           const GeometryOptions &geometryOptions,
                           const AppearanceOptions &appearanceOptions) const {
  bool hasElevation = geometryOptions.elevation > std::numeric_limits<double>::lowest();

  double ele1 = hasElevation ? geometryOptions.elevation : eleProvider_.getElevation(quadKey_, p1.y, p1.x);
  double ele2 = hasElevation ? geometryOptions.elevation : eleProvider_.getElevation(quadKey_, p2.y, p2.x);

  ele1 += NoiseUtils::perlin2D(p1.x, p1.y, geometryOptions.eleNoiseFreq);
  ele2 += NoiseUtils::perlin2D(p2.x, p2.y, geometryOptions.eleNoiseFreq);

  addPlane(mesh, Vector3(p1.x, ele1, p1.y), Vector3(p2.x, ele2, p2.y), geometryOptions, appearanceOptions);
}

void MeshBuilder::addPlane(Mesh &mesh, const Vector3 &p1, const Vector3 &p2,
                           const GeometryOptions &geometryOptions, const AppearanceOptions &appearanceOptions) const {
  auto color =
      appearanceOptions.gradient.evaluate((NoiseUtils::perlin2D(p1.x, p1.y, appearanceOptions.colorNoiseFreq) + 1)/2);
  int index = static_cast<int>(mesh.vertices.size()/3);

  double size = bbox_.width()/appearanceOptions.textureScale;
  // NOTE cannot use original vertices as y is not normalized
  double scaleX = Vector2::distance(Vector2(p2.x, p2.z), Vector2(p1.x, p1.z))/size;
  double scaleY = GeoUtils::getOffset(GeoCoordinate(p1.y, p1.x), geometryOptions.heightOffset)/size;

  const auto topP1 = Vector3(p1.x, p1.y + geometryOptions.heightOffset, p1.z);
  const auto topP2 = Vector3(p2.x, p2.y + geometryOptions.heightOffset, p2.z);

  addVertex(mesh, p1, color, index, Vector2(0, 0));
  addVertex(mesh, p2, color, index + 2, Vector2(scaleX, 0));
  addVertex(mesh, topP2, color, index + 1, Vector2(scaleX, scaleY));
  index += 3;

  addVertex(mesh, topP1, color, index, Vector2(0, scaleY));
  addVertex(mesh, p1, color, index + 2, Vector2(0, 0));
  addVertex(mesh, topP2, color, index + 1, Vector2(scaleX, scaleY));
}

void MeshBuilder::addTriangle(Mesh &mesh,
                              const Vector3 &v0,
                              const Vector3 &v1,
                              const Vector3 &v2,
                              const Vector2 &uv0,
                              const Vector2 &uv1,
                              const Vector2 &uv2,
                              const GeometryOptions &geometryOptions,
                              const AppearanceOptions &appearanceOptions) const {
  auto color =
      appearanceOptions.gradient.evaluate((NoiseUtils::perlin2D(v0.x, v0.z, appearanceOptions.colorNoiseFreq) + 1)/2);
  int startIndex = static_cast<int>(mesh.vertices.size()/3);

  addVertex(mesh, v0, color, startIndex, uv0);
  addVertex(mesh, v1, color, ++startIndex, uv1);
  addVertex(mesh, v2, color, ++startIndex, uv2);

  if (geometryOptions.hasBackSide) {
    // TODO check indices
    addVertex(mesh, v2, color, startIndex, uv0);
    addVertex(mesh, v1, color, ++startIndex, uv1);
    addVertex(mesh, v0, color, ++startIndex, uv2);
  }
}

void MeshBuilder::writeTextureMappingInfo(Mesh &mesh, const AppearanceOptions &appearanceOptions) const {
  if (!mesh.uvMap.empty()) {
    // NOTE: performance optimization: merge neighbours with the same texture.
    auto size = mesh.uvMap.size();
    if (mesh.uvMap[--size]==appearanceOptions.textureRegion.height &&
        mesh.uvMap[--size]==appearanceOptions.textureRegion.width &&
        mesh.uvMap[--size]==appearanceOptions.textureRegion.y &&
        mesh.uvMap[--size]==appearanceOptions.textureRegion.x &&
        mesh.uvMap[--size]==appearanceOptions.textureRegion.atlasHeight &&
        mesh.uvMap[--size]==appearanceOptions.textureRegion.atlasWidth &&
        mesh.uvMap[--size]==appearanceOptions.textureId) {
      mesh.uvMap[--size] = static_cast<int>(mesh.uvs.size());
      return;
    }
  }

  mesh.uvMap.push_back(static_cast<int>(mesh.uvs.size()));
  mesh.uvMap.push_back(appearanceOptions.textureId);
  mesh.uvMap.push_back(appearanceOptions.textureRegion.atlasWidth);
  mesh.uvMap.push_back(appearanceOptions.textureRegion.atlasHeight);
  mesh.uvMap.push_back(appearanceOptions.textureRegion.x);
  mesh.uvMap.push_back(appearanceOptions.textureRegion.y);
  mesh.uvMap.push_back(appearanceOptions.textureRegion.width);
  mesh.uvMap.push_back(appearanceOptions.textureRegion.height);
}

void MeshBuilder::addVertex(Mesh &mesh, const Vector2 &p, double ele, int color, int triIndex, const Vector2 &uv) {
  mesh.vertices.push_back(p.x);
  mesh.vertices.push_back(p.y);
  mesh.vertices.push_back(ele);
  mesh.colors.push_back(color);

  mesh.uvs.push_back(uv.x);
  mesh.uvs.push_back(uv.y);

  mesh.triangles.push_back(triIndex);
}

void MeshBuilder::addVertex(Mesh &mesh, const Vector3 &vertex, int color, int triIndex, const Vector2 &uv) {
  addVertex(mesh, Vector2(vertex.x, vertex.z), vertex.y, color, triIndex, uv);
}
