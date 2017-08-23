using System;
using System.Collections.Generic;
using Assets.Scripts.Core;
using Assets.Scripts.Core.Plugins;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Data;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Utils;
using UtyRx;
using Component = UtyDepend.Component;

namespace Assets.Scripts.Scenes.Import
{
    /// <summary> Scene behaviour which demonstrates how to import data manually. </summary>
    internal class ImportBehavior: MonoBehaviour
    {
        /// <summary> Demonstrates two types of scene. </summary>
        public enum SceneType
        {
            Bird,
            Street
        }

        /// <summary> Path format to map data on disk. </summary>
        /// <remarks> 
        ///    In general, MapCss should adopt tag naming differences for different map data formats,
        ///    e.g OSM xml schema VS mapzen geojson.
        ///    So, assume that you're using default mapcss. Then, if you import map data from OSM, you
        ///    should use LOD = 16, if from mapzen geojson: from 1 to 15. For details, compare mapcss
        ///    rules inside default mapcss.
        /// </remarks>
        private const string MapDataPathFormat = @"../../../../core/test/test_assets/osm/berlin.osm.{0}";

        /// <summary> Start coordinate: Unity's world zero point. </summary>
        /// <remarks>
        ///    If coordinate is not inside imported map data, then data will be fetched from remote server
        ///    based on level of detail.
        /// </remarks>
        private readonly GeoCoordinate _coordinate = new GeoCoordinate(52.5317429, 13.3871987);

        /// <summary> Type of the scene. </summary>
        public SceneType Scene = SceneType.Street;

        private CompositionRoot _compositionRoot;
        private IMapDataStore _mapDataStore;

        void Start()
        {
            // init utymap library
            _compositionRoot = InitTask.Run((container, config) =>
            {
                container
                    .Register(Component.For<Stylesheet>().Use<Stylesheet>(@"mapcss/default/index.mapcss"))
                    .Register(Component.For<MaterialProvider>().Use<MaterialProvider>())
                    .Register(Component.For<GameObjectBuilder>().Use<GameObjectBuilder>())
                    .RegisterInstance<IEnumerable<IElementBuilder>>(new List<IElementBuilder>());
            });
            // store map data store reference to member variable
            _mapDataStore = _compositionRoot.GetService<IMapDataStore>();

            // for demo purpose, disable mesh caching to force import data into memory for every run
            _compositionRoot.GetService<IMapDataLibrary>().DisableCache();

            // get reference for active stylesheet.
            var stylesheet = _compositionRoot.GetService<Stylesheet>();
            // get reference to trace.
            var trace = _compositionRoot.GetService<ITrace>();
            // create tile which represents target region to load.
            var tile = new Tile(
                // create quadkey using coordinate and choose proper LOD
                GeoUtils.CreateQuadKey(_coordinate, Scene == SceneType.Bird ? 14 : 16),
                // provide stylesheet (used to be the same as for import)
                stylesheet,
                // use cartesian projection as we want to build flat world
                new CartesianProjection(_coordinate),
                // use flat elevation (all vertices have zero meters elevation)
                ElevationDataType.Flat,
                // parent for built game objects
                gameObject);

            // import data into memory
            _mapDataStore.Add(
                // define where geoindex is created (in memory, not persistent)
                MapDataStorageType.InMemory,
                // path to map data
                String.Format(MapDataPathFormat, Scene == SceneType.Bird ? "json" : "xml"),
                // stylesheet is used to import only used data and skip unused
                stylesheet,
                // level of detail (zoom) for which map data should be imported
                new Range<int>(16, 16))
                // start import and listen for events.
                .Subscribe(
                    // NOTE progress callback is ignored
                    (progress) => { },
                    // exception is reported
                    (exception) => trace.Error("import", exception, "Cannot import map data"),
                    // once completed, load the corresponding tile
                    () => _mapDataStore.OnNext(tile));
        }
    }
}
