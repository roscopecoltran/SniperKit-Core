using System;
using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;
using UtyMap.Unity.Data;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Tests.Helpers;
using UtyMap.Unity.Utils;

namespace UtyMap.Unity.Tests.Data
{
    [TestFixture(Category = TestHelper.IntegrationTestCategory)]
    public class MapDataEditorTests
    {
        private CompositionRoot _compositionRoot;
        private IMapDataEditor _mapDataEditor;
        private IMapDataStore _dataStore;
        private Stylesheet _stylesheet;
        private IProjection _projection;

        [TestFixtureSetUp]
        public void Setup()
        {
            _compositionRoot = TestHelper.GetCompositionRoot(TestHelper.WorldZeroPoint);
            _mapDataEditor = _compositionRoot.GetService<IMapDataEditor>();
            _dataStore = _compositionRoot.GetService<IMapDataStore>();
            _stylesheet = _compositionRoot.GetService<Stylesheet>();
            _projection = _compositionRoot.GetService<IProjection>();
        }

        [TestFixtureTearDown]
        public void TearDown()
        {
            _compositionRoot.Dispose();
        }

        [Test]
        public void CanAddNode()
        {
            // ARRANGE
            var quadKey = new QuadKey(0, 0, 6);
            var levelOfDetails = new Range<int>(quadKey.LevelOfDetail, quadKey.LevelOfDetail);
            var node = new Element(7,
                new [] { GeoUtils.QuadKeyToBoundingBox(quadKey).Center() },
                new [] { 0.0 },
                // NOTE must be corresponding rule in mapcss.
                new Dictionary<string, string>() { { "kind", "locality" }, {"name", "secret place"} },
                new Dictionary<string, string>());

            // ACT
            _mapDataEditor.Add(MapDataStorageType.InMemory, node, levelOfDetails);

            // ASSERT
            var result = _dataStore.GetResultSync(new Tile(quadKey, _stylesheet, _projection, ElevationDataType.Flat));
            result.Variant.Match(
                e => CompareElements(node, e),
                mesh => { throw new ArgumentException(); });
        }

        private void CompareElements(Element expected, Element actual)
        {
            Assert.AreEqual(expected.Id, actual.Id);
            // geometry
            Assert.AreEqual(expected.Geometry.Length, actual.Geometry.Length);
            for(int i = 0; i < expected.Geometry.Length; ++i)
                Assert.AreEqual(expected.Geometry[i], actual.Geometry[i]);
            // tags
            Assert.AreEqual(expected.Tags.Count, actual.Tags.Count);
            var keys = expected.Tags.Keys.ToArray();
            for (int i = 0; i < keys.Length; ++i)
                Assert.AreEqual(expected.Tags[keys[i]], actual.Tags[keys[i]]); 
        }
    }
}
