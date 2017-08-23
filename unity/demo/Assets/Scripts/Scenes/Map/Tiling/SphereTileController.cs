using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Data;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Utils;
using Object = UnityEngine.Object;

namespace Assets.Scripts.Scenes.Map.Tiling
{
    internal sealed class SphereTileController : TileController
    {
        private readonly Vector3 _origin = Vector3.zero;

        private readonly Transform _camera;
        private readonly float _radius;
        private readonly Dictionary<QuadKey, Tile> _loadedQuadKeys = new Dictionary<QuadKey, Tile>();

        private float _zoom;
        private float _distanceToOrigin;
        private Vector3 _position;
        private Vector3 _rotation;

        public SphereTileController(IMapDataStore dataStore, Stylesheet stylesheet, ElevationDataType elevationType,
            Transform pivot, Range<int> lodRange, float radius) :
            base(dataStore, stylesheet, elevationType, pivot, lodRange)
        {
            _radius = radius;

            LodTree = GetLodTree();
            FieldOfView = 60;
            Projection = new SphericalProjection(radius);

            _camera = pivot.Find("Camera").transform;

            HeightRange = new Range<float>(LodTree.Min, LodTree.Max);
            ResetToDefaults();
        }

        /// <inheritdoc />
        protected override float Height { get { return _distanceToOrigin; } }

        /// <inheritdoc />
        public override Range<float> HeightRange { get; protected set; }

        /// <inheritdoc />
        public override float FieldOfView { get; protected set; }

        /// <inheritdoc />
        public override IProjection Projection { get; protected set; }

        /// <inheritdoc />
        public override float Zoom { get { return _zoom; } }

        /// <inheritdoc />
        public override bool IsAboveMax { get { return HeightRange.Maximum < Height; } }

        /// <inheritdoc />
        public override bool IsBelowMin { get { return HeightRange.Minimum > Height; } }

        /// <inheritdoc />
        public override GeoCoordinate Coordinate
        {
            get
            {
                var latitude = _rotation.x;
                var longitude = (-90 - _rotation.y) % 360;

                if (latitude > 90) latitude -= 360;
                if (longitude < -180) longitude += 360;

                return new GeoCoordinate(latitude, longitude);
            }
        }

        /// <inheritdoc />
        public override void Dispose()
        {
            ResetToDefaults();

            foreach (var quadkey in _loadedQuadKeys.Keys.ToArray())
                SafeDestroy(quadkey);

            UnloadAssets(int.MaxValue);
        }

        /// <inheritdoc />
        public override void Update(Transform target)
        {
            var position = _camera.localPosition;
            var rotation = Pivot.rotation.eulerAngles;

            if (Vector3.Distance(_rotation, rotation) < float.Epsilon &&
                Vector3.Distance(position, _position) < float.Epsilon)
                return;

            _position = position;
            _rotation = rotation;
            _distanceToOrigin = Vector3.Distance(_position, _origin);
            
            if (IsAboveMax || IsBelowMin)
                return;

            _zoom = CalculateZoom(_distanceToOrigin);

            if (_loadedQuadKeys.Any())
                BuildIfNecessary(target);
            else
                BuildInitial(target);
        }

        #region LOD calculations

        private RangeTree<float, int> GetLodTree()
        {
            var lodTree = new RangeTree<float, int>();

            for (int lod = LodRange.Minimum; lod <= LodRange.Maximum; ++lod)
            {
                lodTree.Add(_radius + _radius * Mathf.Pow(2, 1 - lod),
                            _radius + _radius * Mathf.Pow(2, 2 - lod),
                            lod);
            }

            lodTree.Rebuild();

            return lodTree;
        }

        #endregion

        #region Tile processing

        /// <summary> Builds quadkeys if necessary. Decision is based on visible quadkey and lod level. </summary>
        private void BuildIfNecessary(Transform planet)
        {
            var lod = (int) _zoom;

            var actualGameObject = GetActual(planet, Coordinate, lod);
            if (actualGameObject == planet)
                return;

            var actualQuadKey = QuadKey.FromString(actualGameObject.name);
            var actualName = actualGameObject.name;

            var parent = planet;
            var quadKeys = new List<QuadKey>();

            // zoom in
            int disposedTiles = 0;
            if (actualQuadKey.LevelOfDetail < lod)
            {
                quadKeys.AddRange(GetChildren(actualQuadKey));
                var oldParent = actualGameObject.transform.parent;
                disposedTiles = SafeDestroy(actualQuadKey, actualName) ? 1 : 0;

                parent = new GameObject(actualName).transform;
                parent.transform.parent = oldParent;
            }
            // zoom out
            else if (actualQuadKey.LevelOfDetail > lod)
            {
                string name = actualName.Substring(0, actualName.Length - 1);
                var quadKey = QuadKey.FromString(name);
                // destroy all siblings
                foreach (var child in GetChildren(quadKey))
                    disposedTiles += SafeDestroy(child, child.ToString()) ? 1 : 0;
                // destroy current as it might be just placeholder.
                SafeDestroy(actualQuadKey, name);
                parent = GetParent(planet, quadKey);
                quadKeys.Add(quadKey);
            }

            UnloadAssets(disposedTiles);
            BuildQuadKeys(parent, quadKeys);
        }

        /// <summary> Builds planet on initial lod. </summary>
        private void BuildInitial(Transform planet)
        {
            var quadKeys = new List<QuadKey>();
            var maxQuad = GeoUtils.CreateQuadKey(new GeoCoordinate(-89.99, 179.99), LodRange.Minimum);
            for (int y = 0; y <= maxQuad.TileY; ++y)
                for (int x = 0; x <= maxQuad.TileX; ++x)
                    quadKeys.Add(new QuadKey(x, y, LodRange.Minimum));

            BuildQuadKeys(planet, quadKeys);
        }

        /// <summary> Builds given quadkeys. </summary>
        private void BuildQuadKeys(Transform planet, IEnumerable<QuadKey> quadKeys)
        {
            foreach (var quadKey in quadKeys)
            {
                if (_loadedQuadKeys.ContainsKey(quadKey))
                    continue;

                var tileGameObject = new GameObject(quadKey.ToString());
                tileGameObject.transform.parent = planet;
                var tile = CreateTile(quadKey, tileGameObject);
                _loadedQuadKeys.Add(quadKey, tile);
                LoadTile(tile);
            }
        }

        /// <summary> Gets childrent for quadkey. </summary>
        private IEnumerable<QuadKey> GetChildren(QuadKey quadKey)
        {
            // TODO can be optimized to avoid string allocations.
            var quadKeyName = quadKey.ToString();
            yield return QuadKey.FromString(quadKeyName + "0");
            yield return QuadKey.FromString(quadKeyName + "1");
            yield return QuadKey.FromString(quadKeyName + "2");
            yield return QuadKey.FromString(quadKeyName + "3");
        }

        /// <summary> Gets actual loaded quadkey's gameobject for given coordinate. </summary>
        private Transform GetActual(Transform planet, GeoCoordinate coordinate, int lod)
        {
            var expectedQuadKey = GeoUtils.CreateQuadKey(coordinate, lod);

            if (_loadedQuadKeys.ContainsKey(expectedQuadKey))
                return _loadedQuadKeys[expectedQuadKey].GameObject.transform;

            var expectedGameObject = GameObject.Find(expectedQuadKey.ToString());
            return expectedGameObject == null
                ? GetParent(planet, expectedQuadKey)           // zoom in
                : GetLastParent(expectedGameObject.transform); // zoom out or pan
        }

        /// <summary> Gets parent game object for given quadkey. Creates hierarchy if necessary. </summary>
        private Transform GetParent(Transform planet, QuadKey quadKey)
        {
            // recursion end
            if (quadKey.LevelOfDetail <= LodRange.Minimum)
                return planet;

            string quadKeyName = quadKey.ToString();
            string parentName = quadKeyName.Substring(0, quadKeyName.Length - 1);
            var parent = GameObject.Find(parentName);
            return parent != null
                ? parent.transform
                : GetParent(planet, QuadKey.FromString(parentName));
        }

        /// <summary> Gets the last descendant game object with children. </summary>
        private Transform GetLastParent(Transform transform)
        {
            return transform.transform.childCount == 0
                ? transform.transform.parent.transform
                : GetLastParent(transform.GetChild(0).transform);
        }

        #endregion

        /// <summary> Destroys gameobject by its name if it exists. </summary>
        private bool SafeDestroy(QuadKey quadKey, string name = null)
        {
            if (_loadedQuadKeys.ContainsKey(quadKey))
            {
                _loadedQuadKeys[quadKey].Dispose();
                _loadedQuadKeys.Remove(quadKey);
                return true;
            }

            var go = GameObject.Find(name);
            if (go != null)
                Object.Destroy(go);
            return false;
        }

        private void ResetToDefaults()
        {
            _position = new Vector3(float.MinValue, float.MinValue, float.MinValue);
            _rotation = new Vector3(float.MinValue, float.MinValue, float.MinValue);
            _distanceToOrigin = HeightRange.Minimum + (HeightRange.Maximum - HeightRange.Minimum) / 2;
            _zoom = 0;
        }
    }
}
