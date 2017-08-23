#include "builders/MeshCache.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

#define CHECK_CLOSE_COLLECTION(aa, bb, tolerance) { \
    using std::distance; \
    using std::begin; \
    using std::end; \
    auto a = begin(aa), ae = end(aa); \
    auto b = begin(bb); \
    BOOST_REQUIRE_EQUAL(distance(a, ae), distance(b, end(bb))); \
    for(; a != ae; ++a, ++b) { \
        BOOST_CHECK_CLOSE(*a, *b, tolerance); \
        } \
}

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::math;
using namespace utymap::mapcss;
using namespace utymap::tests;

namespace {
const double Precision = 1e-5;
const QuadKey quadKey = QuadKey(1, 0, 0);
const std::string stylesheet = "node|z1[any], way|z1[any], area|z1[any], relation|z1[any] { clip: false; }";

std::string getCacheDir(const StyleProvider &styleProvider) {
  return std::string("cache/") + styleProvider.getTag() + "/1";
}

BuilderContext wrap(const StyleProvider &styleProvider,
                    const BuilderContext &context,
                    MeshCache &cache) {
  boost::filesystem::create_directories(getCacheDir(styleProvider));
  return cache.wrap(context);
}

struct Builders_MeshCacheFixture {
  void meshCallback(const Mesh &mesh) {
    lastMesh_.name = mesh.name;
    lastMesh_.vertices = mesh.vertices;
    lastMesh_.triangles = mesh.triangles;
    lastMesh_.colors = mesh.colors;
    lastMesh_.uvs = mesh.uvs;
    lastMesh_.uvMap = mesh.uvMap;
  }

  void elementCallback(const Element &element) {
    lastId_ = element.id;
  }

  void resetData() {
    lastId_ = 0;
    lastMesh_.name.clear();
    lastMesh_.clear();
  }

  Builders_MeshCacheFixture() :
      cache_("", "mesh"),
      origContext(quadKey,
                  *dependencyProvider.getStyleProvider(stylesheet),
                  *dependencyProvider.getStringTable(),
                  *dependencyProvider.getElevationProvider(),
                  std::bind(&Builders_MeshCacheFixture::meshCallback, this, std::placeholders::_1),
                  std::bind(&Builders_MeshCacheFixture::elementCallback, this, std::placeholders::_1),
                  dependencyProvider.getCancellationToken()),
      wrapContext(wrap(*dependencyProvider.getStyleProvider(), origContext, cache_)),
      lastMesh_("") {
    resetData();
  }

  ~Builders_MeshCacheFixture() {
    auto filePath = getCacheDir(*dependencyProvider.getStyleProvider()) + "/0.mesh";
    boost::filesystem::remove(filePath);
  }

  void assertStoreAndFetch(const Element &element) {
    // Assert that original callback is called
    wrapContext.elementCallback(element);
    BOOST_CHECK_EQUAL(lastId_, element.id);

    // Release context and reset actual values
    cache_.unwrap(wrapContext);
    resetData();

    // Assert that element is read back
    cache_.fetch(origContext);
    BOOST_CHECK_EQUAL(lastId_, element.id);
  }

  void assertStoreAndFetch(const Mesh &mesh) {
    // Assert that original callback is called
    wrapContext.meshCallback(mesh);
    assertMesh(mesh);

    // Release context and reset actual values
    cache_.unwrap(wrapContext);
    resetData();

    // Assert that element is read back
    cache_.fetch(origContext);
    assertMesh(mesh);
  }

  void assertMesh(const Mesh &mesh) const {
    BOOST_CHECK_EQUAL(lastMesh_.name, mesh.name);
    CHECK_CLOSE_COLLECTION(lastMesh_.vertices, mesh.vertices, Precision);
    CHECK_CLOSE_COLLECTION(lastMesh_.uvs, mesh.uvs, Precision);
    BOOST_CHECK_EQUAL_COLLECTIONS(lastMesh_.triangles.begin(), lastMesh_.triangles.end(),
      mesh.triangles.begin(), mesh.triangles.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(lastMesh_.colors.begin(), lastMesh_.colors.end(),
      mesh.colors.begin(), mesh.colors.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(lastMesh_.uvMap.begin(), lastMesh_.uvMap.end(),
      mesh.uvMap.begin(), mesh.uvMap.end());
  }

  DependencyProvider dependencyProvider;
  MeshCache cache_;
  BuilderContext origContext;
  BuilderContext wrapContext;

  std::uint64_t lastId_;
  Mesh lastMesh_;
};
}

BOOST_FIXTURE_TEST_SUITE(Builders_MeshCache, Builders_MeshCacheFixture)

BOOST_AUTO_TEST_CASE(GivenNode_WhenStoreAndFetch_ThenItIsStoredAndReadBack) {
  Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 1, {{"any", "true"}});

  assertStoreAndFetch(node);
}

BOOST_AUTO_TEST_CASE(GivenWay_WhenStoreAndFetch_ThenItIsStoredAndReadBack) {
  Way way =
      ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), 7, {{"any", "true"}}, {{1, -1}, {5, -5}});

  assertStoreAndFetch(way);
}

BOOST_AUTO_TEST_CASE(GivenArea_WhenStoreAndFetch_ThenItIsStoredAndReadBack) {
  Area area = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(),
                                                7,
                                                {{"any", "true"}},
                                                {{1, -1}, {5, -5}, {10, -10}});

  assertStoreAndFetch(area);
}

BOOST_AUTO_TEST_CASE(GivenRelation_WhenStoreAndFetch_ThenItIsStoredAndReadBack) {
  Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 1, {{"n", "1"}});
  node.coordinate = {0.5, -0.5};
  Way way = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), 2, {{"w", "2"}}, {{1, -1}, {2, -2}});
  Area area = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(),
                                                3,
                                                {{"a", "3"}},
                                                {{3, -3}, {4, -4}, {5, -5}});
  Relation relation = ElementUtils::createElement<Relation>(*dependencyProvider.getStringTable(), 4, {{"any", "true"}});
  relation.elements.push_back(std::make_shared<Node>(node));
  relation.elements.push_back(std::make_shared<Way>(way));
  relation.elements.push_back(std::make_shared<Area>(area));

  assertStoreAndFetch(relation);
}

BOOST_AUTO_TEST_CASE(GivenMesh_WhenStoreAndFetch_ThenItIsStoredAndReadBack) {
  Mesh mesh("My mesh");
  mesh.vertices.assign({1, 2, 3, 4.5, 5.555555, 6.6666666});
  mesh.triangles.assign({1, 2, 3, 4});
  mesh.colors.assign({4, 3, 2, 1});
  mesh.uvs.assign({0.1, 0.2, 0.3, 0.4});
  mesh.uvMap.assign({1, 2, 3});

  assertStoreAndFetch(mesh);
}

BOOST_AUTO_TEST_SUITE_END()
