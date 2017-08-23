using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Data;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Utils;

namespace Assets.Scripts.Scenes.Map.Tiling
{
    /// <summary>  </summary>
    internal sealed class SurfaceTileController : TileController
    {
        private readonly float _scale;

        private float _zoom;
        private Vector3 _position;
        private GeoCoordinate _geoOrigin;
        private Dictionary<QuadKey, Tile> _loadedQuadKeys = new Dictionary<QuadKey, Tile>();

        public SurfaceTileController(IMapDataStore dataStore, Stylesheet stylesheet, ElevationDataType elevationType,
            Transform pivot, Range<int> lodRange, GeoCoordinate origin, float scale, float maxDistance) :
            base(dataStore, stylesheet, elevationType, pivot, lodRange)
        {
            _scale = scale;
            _geoOrigin = origin;

            LodTree = GetLodTree(pivot.Find("Camera").GetComponent<Camera>().aspect, maxDistance);
            Projection = CreateProjection();

            HeightRange = new Range<float>(LodTree.Min, LodTree.Max);
            ResetToDefaults();
        }

        /// <inheritdoc />
        protected override float Height { get { return _position.y; } }

        /// <inheritdoc />
        public override Range<float> HeightRange { get; protected set; }

        /// <inheritdoc />
        public override float FieldOfView { get; protected set; }

        /// <inheritdoc />
        public override IProjection Projection { get; protected set; }

        /// <inheritdoc />
        public override float Zoom { get { return _zoom; } }

        /// <inheritdoc />
        public override bool IsAboveMax { get { return HeightRange.Maximum < _position.y; } }

        /// <inheritdoc />
        public override bool IsBelowMin { get { return HeightRange.Minimum > _position.y; } }

        /// <inheritdoc />
        public override GeoCoordinate Coordinate {
            get
            {
                return GeoUtils.ToGeoCoordinate(_geoOrigin, new Vector2(_position.x, _position.z) / _scale);
            }
        }

        /// <inheritdoc />
        public override void Dispose()
        {
            ResetToDefaults();

            foreach (var tile in _loadedQuadKeys.Values.ToArray())
                tile.Dispose();

            UnloadAssets(int.MaxValue);
        }

        /// <summary> Moves view center to given coordinate. </summary>
        public void MoveGeoOrigin(GeoCoordinate origin)
        {
            _geoOrigin = origin;
            Projection = CreateProjection();
        }

        /// <inheritdoc />
        public override void Update(Transform target)
        {
            var position = Pivot.localPosition;

            if (Vector3.Distance(position, _position) < float.Epsilon)
                return;

            var oldLod = (int)_zoom;

            _position = position;
            _zoom = CalculateZoom(_position.y);

            if (IsAboveMax || IsBelowMin)
                return;

            Build(target, oldLod);
        }

        #region Tile processing

        // TODO call this method when tile is moved too far.
        /// <summary> Moves geo origin to specific world position. </summary>
        private void MoveWorldOrigin(Vector3 position)
        {
            var geoOrigin = GeoUtils.ToGeoCoordinate(_geoOrigin, new Vector2(position.x, position.z) / _scale);
            MoveGeoOrigin(geoOrigin);
        }

        /// <summary> Builds quadkeys if necessary. Decision is based on current position and lod level. </summary>
        private void Build(Transform parent, int oldLod)
        {
            var currentLod = LodTree[_position.y].Single().Value;

            var currentQuadKey = GetQuadKey(currentLod);

            // zoom in/out
            if (oldLod != currentLod)
            {
                foreach (var tile in _loadedQuadKeys.Values)
                    tile.Dispose();

                UnloadAssets(_loadedQuadKeys.Count);
                _loadedQuadKeys.Clear();

                foreach (var quadKey in GetNeighbours(currentQuadKey))
                    BuildQuadKey(parent, quadKey);
            }
            // pan
            else
            {
                var quadKeys = new HashSet<QuadKey>(GetNeighbours(currentQuadKey));
                var newlyLoadedQuadKeys = new Dictionary<QuadKey, Tile>();

                foreach (var quadKey in quadKeys)
                    newlyLoadedQuadKeys.Add(quadKey, _loadedQuadKeys.ContainsKey(quadKey)
                        ? _loadedQuadKeys[quadKey]
                        : BuildQuadKey(parent, quadKey));

                int tilesDisposed = 0;
                foreach (var quadKeyPair in _loadedQuadKeys)
                    if (!quadKeys.Contains(quadKeyPair.Key))
                    {
                        ++tilesDisposed;
                        quadKeyPair.Value.Dispose();
                    }

                UnloadAssets(tilesDisposed);
                _loadedQuadKeys = newlyLoadedQuadKeys;
            }
        }

        /// <summary> Get tiles surrounding given. </summary>
        private IEnumerable<QuadKey> GetNeighbours(QuadKey quadKey)
        {
            yield return new QuadKey(quadKey.TileX, quadKey.TileY, quadKey.LevelOfDetail);

            yield return new QuadKey(quadKey.TileX - 1, quadKey.TileY, quadKey.LevelOfDetail);
            yield return new QuadKey(quadKey.TileX - 1, quadKey.TileY + 1, quadKey.LevelOfDetail);
            yield return new QuadKey(quadKey.TileX, quadKey.TileY + 1, quadKey.LevelOfDetail);
            yield return new QuadKey(quadKey.TileX + 1, quadKey.TileY + 1, quadKey.LevelOfDetail);
            yield return new QuadKey(quadKey.TileX + 1, quadKey.TileY, quadKey.LevelOfDetail);
            yield return new QuadKey(quadKey.TileX + 1, quadKey.TileY - 1, quadKey.LevelOfDetail);
            yield return new QuadKey(quadKey.TileX, quadKey.TileY - 1, quadKey.LevelOfDetail);
            yield return new QuadKey(quadKey.TileX - 1, quadKey.TileY - 1, quadKey.LevelOfDetail);
        }

        /// <summary> Build specific quadkey. </summary>
        private Tile BuildQuadKey(Transform parent, QuadKey quadKey)
        {
            var tileGameObject = new GameObject(quadKey.ToString());
            tileGameObject.transform.parent = parent.transform;
            var tile = CreateTile(quadKey, tileGameObject);
            _loadedQuadKeys.Add(quadKey, tile);
            LoadTile(tile);
            return tile;
        }

        /// <summary> Gets quadkey for position. </summary>
        private QuadKey GetQuadKey(int lod)
        {
            var currentPosition = GeoUtils.ToGeoCoordinate(_geoOrigin, new Vector2(_position.x, _position.z) / _scale);
            return GeoUtils.CreateQuadKey(currentPosition, lod);
        }

        #endregion

        #region Lod calculations

        /// <summary> Gets range (interval) tree with LODs </summary>
        private RangeTree<float, int> GetLodTree(float cameraAspect, float maxDistance)
        {
            const float sizeRatio = 0.5f;
            var tree = new RangeTree<float, int>();

            var aspectRatio = sizeRatio * (Screen.height < Screen.width ? 1 / cameraAspect : 1);
            FieldOfView = GetFieldOfView(GeoUtils.CreateQuadKey(_geoOrigin, LodRange.Minimum), maxDistance, aspectRatio);

            if (LodRange.Minimum == LodRange.Maximum)
                tree.Add(0, maxDistance, LodRange.Minimum);
            else
            {
                for (int lod = LodRange.Minimum; lod <= LodRange.Maximum; ++lod)
                {
                    var frustumHeight = GetFrustumHeight(GeoUtils.CreateQuadKey(_geoOrigin, lod), aspectRatio);
                    var distance = frustumHeight * 0.5f / Mathf.Tan(FieldOfView * 0.5f * Mathf.Deg2Rad);
                    tree.Add(distance, maxDistance, lod);
                    maxDistance = distance - float.Epsilon;
                }
            }

            tree.Rebuild();

            return tree;
        }

        /// <summary> Gets height of camera's frustum. </summary>
        private float GetFrustumHeight(QuadKey quadKey, float aspectRatio)
        {
            return GetGridHeight(quadKey) * aspectRatio;
        }

        /// <summary> Gets field of view for given quadkey and distance. </summary>
        private float GetFieldOfView(QuadKey quadKey, float distance, float aspectRatio)
        {
            return 2.0f * Mathf.Rad2Deg * Mathf.Atan(GetFrustumHeight(quadKey, aspectRatio) / distance);
        }

        /// <summary> Get side size in meters of grid consists of 9 quadkeys. </summary>
        private float GetGridHeight(QuadKey quadKey)
        {
            var bbox = GeoUtils.QuadKeyToBoundingBox(quadKey);
            var bboxHeight = bbox.MaxPoint.Latitude - bbox.MinPoint.Latitude;
            var minPoint = new GeoCoordinate(bbox.MinPoint.Latitude - bboxHeight, bbox.MinPoint.Longitude);
            var maxPoint = new GeoCoordinate(bbox.MinPoint.Latitude + bboxHeight, bbox.MaxPoint.Longitude);
            return (float) GeoUtils.Distance(minPoint, maxPoint) * _scale;
        }

        #endregion

        private void ResetToDefaults()
        {
            _position = new Vector3(float.MinValue, float.MinValue, float.MinValue);
            _zoom = 0;
        }

        /// <summary> Gets projection for current georigin and scale. </summary>
        private IProjection CreateProjection()
        {
            IProjection projection = new CartesianProjection(_geoOrigin);
            return _scale > 0
                ? new ScaledProjection(projection, _scale)
                : projection;
        }
    }
}
