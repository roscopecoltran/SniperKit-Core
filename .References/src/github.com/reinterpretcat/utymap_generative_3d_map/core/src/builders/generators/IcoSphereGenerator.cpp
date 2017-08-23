#include "builders/generators/IcoSphereGenerator.hpp"

using namespace utymap::builders;
using namespace utymap::math;

namespace {
/// A helper struct for calculations.
struct TriangleIndices {
  std::size_t V1, V2, V3;
  TriangleIndices(std::size_t v1, std::size_t v2, std::size_t v3) :
      V1(v1), V2(v2), V3(v3) {
  }
};

/// Creates 12 vertices of a icosahedron
std::vector<Vector3> createVertexList() {
  const static double t = (1 + std::sqrt(5))/2;
  const static std::vector<Vector3> vertexList =
      {
          Vector3(-1, t, 0).normalized(),
          Vector3(1, t, 0).normalized(),
          Vector3(-1, -t, 0).normalized(),
          Vector3(1, -t, 0).normalized(),

          Vector3(0, -1, t).normalized(),
          Vector3(0, 1, t).normalized(),
          Vector3(0., -1, -t).normalized(),
          Vector3(0, 1, -t).normalized(),

          Vector3(t, 0, -1).normalized(),
          Vector3(t, 0, 1).normalized(),
          Vector3(-t, 0, -1).normalized(),
          Vector3(-t, 0, 1).normalized(),
      };

  return vertexList;
}

// Creates 20 triangles of the icosahedron
const std::vector<TriangleIndices> &getSphereFaces() {
  const static std::vector<TriangleIndices> faces
      {
          TriangleIndices(0, 11, 5),
          TriangleIndices(0, 5, 1),
          TriangleIndices(0, 1, 7),
          TriangleIndices(0, 7, 10),
          TriangleIndices(0, 10, 11),
          TriangleIndices(1, 5, 9),
          TriangleIndices(5, 11, 4),
          TriangleIndices(11, 10, 2),
          TriangleIndices(10, 7, 6),
          TriangleIndices(7, 1, 8),
          TriangleIndices(3, 9, 4),
          TriangleIndices(3, 4, 2),
          TriangleIndices(3, 2, 6),
          TriangleIndices(3, 6, 8),
          TriangleIndices(3, 8, 9),
          TriangleIndices(4, 9, 5),
          TriangleIndices(2, 4, 11),
          TriangleIndices(6, 2, 10),
          TriangleIndices(8, 6, 7),
          TriangleIndices(9, 8, 1)
      };

  return faces;
}

const std::vector<TriangleIndices> &getSemiSphereFaces() {
  const static std::vector<TriangleIndices> faces
      {
          TriangleIndices(0, 11, 5),
          TriangleIndices(0, 5, 1),
          TriangleIndices(0, 1, 7),
          TriangleIndices(0, 7, 10),
          TriangleIndices(0, 10, 11),
          TriangleIndices(1, 5, 9),
          TriangleIndices(5, 11, 4),
          TriangleIndices(10, 7, 6),
          TriangleIndices(7, 1, 8),
          TriangleIndices(4, 9, 5),
          TriangleIndices(8, 6, 7),
          TriangleIndices(9, 8, 1)
      };

  return faces;
}
}

void IcoSphereGenerator::generate() {
  vertexList_ = createVertexList();
  const auto &faces = isSemiSphere_ ? getSemiSphereFaces() : getSphereFaces();

  // refine triangles
  std::vector<TriangleIndices> meshFaces;
  for (int i = 0; i < recursionLevel_; i++) {
    std::vector<TriangleIndices> faces2;
    faces2.reserve(faces.size()*4);
    for (const auto &tri : faces) {
      // replace triangle by 4 triangles
      auto a = getMiddlePoint(tri.V1, tri.V2);
      auto b = getMiddlePoint(tri.V2, tri.V3);
      auto c = getMiddlePoint(tri.V3, tri.V1);

      faces2.push_back(TriangleIndices(tri.V1, a, c));
      faces2.push_back(TriangleIndices(tri.V2, b, a));
      faces2.push_back(TriangleIndices(tri.V3, c, b));
      faces2.push_back(TriangleIndices(a, b, c));
    }
    meshFaces = std::move(faces2);
  }

  // generate mesh
  for (const auto &face : meshFaces) {
    addTriangle(
        translate(scale(vertexList_[face.V1]) + center_),
        translate(scale(vertexList_[face.V2]) + center_),
        translate(scale(vertexList_[face.V3]) + center_));
  }

  // clear state to allow reuse
  middlePointIndexCache_.clear();
  vertexList_.clear();
}

std::size_t IcoSphereGenerator::getMiddlePoint(std::size_t p1, std::size_t p2) {
  // first check if we have it already
  bool firstIsSmaller = p1 < p2;
  std::uint64_t smallerIndex = firstIsSmaller ? p1 : p2;
  std::uint64_t greaterIndex = firstIsSmaller ? p2 : p1;
  std::uint64_t key = (smallerIndex << 32) + greaterIndex;

  auto ret = middlePointIndexCache_.find(key);
  if (ret!=middlePointIndexCache_.end())
    return ret->second;

  // not in cache, calculate it
  Vector3 point1 = vertexList_[p1];
  Vector3 point2 = vertexList_[p2];
  Vector3 middle(
      (point1.x + point2.x)/2,
      (point1.y + point2.y)/2,
      (point1.z + point2.z)/2);

  // add vertex makes sure point is on unit sphere
  std::size_t size = vertexList_.size();
  vertexList_.push_back(middle.normalized());

  // store it, return index
  middlePointIndexCache_.insert(std::make_pair(key, size));

  return size;
}
