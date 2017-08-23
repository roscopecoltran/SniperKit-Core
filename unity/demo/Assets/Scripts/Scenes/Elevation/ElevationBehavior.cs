using System.Collections.Generic;
using Assets.Scripts.Core;
using Assets.Scripts.Core.Plugins;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Data;
using UtyMap.Unity.Utils;
using Component = UtyDepend.Component;

namespace Assets.Scripts.Scenes.Elevation
{
    /// <summary> Scene behaviour which demonstrates non-flat elevation in action. </summary>
    internal class ElevationBehavior: MonoBehaviour
    {
        public double Latitude = 47.1411127;
        public double Longitude = 9.5212054;

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

            // get reference for active stylesheet
            var stylesheet = _compositionRoot.GetService<Stylesheet>();
            // define level of detail
            const int levelOfDetail = 14;
            // create center coordinate;
            var coordinate = (new GeoCoordinate(Latitude, Longitude));
            // create "center" tile
            var center = GeoUtils.CreateQuadKey(coordinate, levelOfDetail);

            // load multiply tiles at once
            for (var tileX = center.TileX - 1; tileX <= center.TileX + 1; ++tileX)
            for (var tileY = center.TileY - 1; tileY <= center.TileY + 1; ++tileY)
            {
                var quadKey = new QuadKey(tileX, tileY, levelOfDetail);
                var parent = new GameObject(quadKey.ToString());
                parent.transform.SetParent(gameObject.transform);
                _mapDataStore.OnNext(new Tile(
                    // quadkey to load.
                    quadKey,
                    // provide stylesheet
                    stylesheet,
                    // use cartesian projection as we want to build flat world
                    new CartesianProjection(coordinate),
                    // use grid elevation: uses mapzen servers by default,
                    // stores elevation in simple NxN grid format.
                    ElevationDataType.Grid,
                    // parent for built game objects
                    parent));
            }
        }
    }
}
