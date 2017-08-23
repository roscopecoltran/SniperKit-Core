using NUnit.Framework;
using UtyMap.Unity.Data;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Tests.Helpers;
using UtyMap.Unity.Utils;
using UtyRx;

namespace UtyMap.Unity.Tests.Data
{
    [TestFixture(Category = TestHelper.IntegrationTestCategory)]
    public class TileLoadingTests
    {
        private readonly GeoCoordinate _worldZeroPoint = TestHelper.WorldZeroPoint;
        private CompositionRoot _compositionRoot;
        private IMapDataStore _mapDataStore;
        private Stylesheet _stylesheet;
        private IProjection _projection;
        private bool _isCalled;

        [TestFixtureSetUp]
        public void Setup()
        {
            // initialize services
            _compositionRoot = TestHelper.GetCompositionRoot(_worldZeroPoint);
            _stylesheet = _compositionRoot.GetService<Stylesheet>();
            _projection = _compositionRoot.GetService<IProjection>();
            // get local references
            _mapDataStore = _compositionRoot.GetService<IMapDataStore>();
            _isCalled = false;
        }

        [TestFixtureTearDown]
        public void TearDown()
        {
            _compositionRoot.Dispose();
        }

        [Test(Description = "This test loads 1 tile at zoom level 16.")]
        public void CanLoadTilesAtDetailedLevelOfDetails()
        {
            // ARRANGE
            int lod = 16;
            SetupMapData(TestHelper.BerlinPbfData, lod);
            var count = 1;
            var centerQuadKey = new QuadKey(35205, 21489, lod);

            // ACT & ASSERT
            TestQuadKeys(centerQuadKey, count, lod);
        }

        [Test(Description = "This test loads 1 tile at zoom level 14.")]
        public void CanLoadTilesAtBirdEyeLevelOfDetails()
        {
            // ARRANGE
            int lod = 14;
            SetupMapData(TestHelper.MoscowJsonData, lod);
            var count = 1;
            var centerQuadKey = GeoUtils.CreateQuadKey(new GeoCoordinate(55.75363, 37.62027), lod);

            // ACT & ASSERT
            TestQuadKeys(centerQuadKey, count, lod);
        }

        #region Private members

        /// <summary> Setup test map data. </summary>
        private void SetupMapData(string mapDataPath, int lod)
        {
            var range = new Range<int>(lod, lod);
            _compositionRoot
                .GetService<IMapDataStore>()
                .Add(MapDataStorageType.InMemory, mapDataPath, _stylesheet, range);
        }

        private void TestQuadKeys(QuadKey centerQuadKey, int count, int lod)
        {
            for (var y = 0; y < count; ++y)
            for (var x = 0; x < count; ++x)
            {
                var quadKey = new QuadKey(centerQuadKey.TileX + x, centerQuadKey.TileY + y, lod);
                var tile = new Tile(quadKey, _stylesheet, _projection, ElevationDataType.Flat);
                AssertResult(_mapDataStore.GetResultSync(tile));
            }
            Assert.IsTrue(_isCalled);
        }

        /// <summary> Checks whether result satisfied minimal correctness critirea. </summary>
        private void AssertResult(MapData result)
        {
            _isCalled = true;
            var elements = 0;
            var meshes = 0;
            result.Variant.Match(
                element =>
                {
                    Assert.Greater(element.Geometry.Length, 0);
                    ++elements;
                },
                mesh =>
                {
                    Assert.Greater(mesh.Vertices.Length, 0);
                    Assert.Greater(mesh.Triangles.Length, 0);
                    Assert.Greater(mesh.Colors.Length, 0);
                    ++meshes;
                });

            Assert.Greater(elements + meshes, 0);
        }

        #endregion
    }
}
