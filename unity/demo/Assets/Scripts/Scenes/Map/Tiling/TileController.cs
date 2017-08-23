using System;
using System.Linq;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Data;
using UtyMap.Unity.Infrastructure.Primitives;

namespace Assets.Scripts.Scenes.Map.Tiling
{
    internal abstract class TileController : IDisposable
    {
        private readonly IMapDataStore _dataStore;
        private readonly Stylesheet _stylesheet;
        private readonly ElevationDataType _elevationType;

        private int _disposedTilesCounter = 0;

        /// <summary> Contains LOD values mapped for height ranges. </summary>
        protected RangeTree<float, int> LodTree;

        /// <summary> Gets height in scaled coordinates. </summary>
        protected abstract float Height { get; }

        /// <summary> Scaled height range. </summary>
        public abstract Range<float> HeightRange { get; protected set; }

        /// <summary> Pivot. </summary>
        public readonly Transform Pivot;

        /// <summary> Level of detail range handled by the controller. </summary>
        public readonly Range<int> LodRange;

        /// <summary> Gets camera's field of view. </summary>
        public abstract float FieldOfView { get; protected set; }

        /// <summary> Current used projection. </summary>
        public abstract IProjection Projection { get; protected set; }

        /// <summary> Gets current zoom level. </summary>
        public abstract float Zoom { get; }

        /// <summary> Is above maximum zoom level. </summary>
        public abstract bool IsAboveMax { get; }

        /// <summary> Is belove minimum zoom level </summary>
        public abstract bool IsBelowMin { get; }

        // <summary> Gets current geo coordinate. </summary>
        public abstract GeoCoordinate Coordinate { get; }

        /// <summary> Updates target. </summary>
        public abstract void Update(Transform target);

        /// <inheritdoc />
        public abstract void Dispose();

        protected TileController(IMapDataStore dataStore, Stylesheet stylesheet, ElevationDataType elevationType,
            Transform pivot, Range<int> lodRange)
        {
            _dataStore = dataStore;
            _stylesheet = stylesheet;
            _elevationType = elevationType;

            Pivot = pivot;
            LodRange = lodRange;
        }

        #region Height calculation

        /// <summary> Calculates height in scaled world coordinates for given zoom using height provided. </summary>
        public float GetHeight(float zoom)
        {
            int lod = (int) Math.Floor(zoom);
            return LodRange.Contains(lod)
                ? InterpolateHeight(zoom, lod)
                : ExtrapolateHeight(zoom, lod);
        }

        private float InterpolateHeight(float zoom, int lod)
        {
            var minHeight = float.MinValue;
            var maxHeight = float.MinValue;

            foreach (var rangeValuePair in LodTree)
            {
                if (rangeValuePair.Value == lod)
                {
                    minHeight = rangeValuePair.From;
                    maxHeight = rangeValuePair.To;
                    break;
                }
            }

            // NOTE: we clamp with some tolerance to prevent issues with float precision when
            // the distance is huge (planet level). Theoretically, double type will fix 
            // the problem but it will force to use casting to float in multiple places.
            var range = maxHeight - minHeight;
            var tolerance = range * 0.00001f;
            return Mathf.Clamp(minHeight + range * (lod + 1 - zoom), minHeight + tolerance, maxHeight - tolerance);
        }

        private float ExtrapolateHeight(float zoom, int lod)
        {
            bool isZoomIn = lod > LodRange.Maximum;

            // NOTE prevent no height yet known case.
            var currentHeight = Math.Abs(Height - float.MinValue) < float.Epsilon
                ? (isZoomIn ? HeightRange.Minimum : HeightRange.Maximum)
                : Height;
            
            var distanceInsideSpace = isZoomIn
                ? currentHeight - HeightRange.Minimum
                : HeightRange.Maximum - currentHeight;

            var zoomInsideSpace = (isZoomIn
                ? LodRange.Maximum - Zoom + 1
                : Zoom - LodRange.Minimum) ;

            var zoomOutsideSpace = (isZoomIn
                ? zoom - LodRange.Maximum - 1
                : LodRange.Minimum - zoom) ;

            var distanceOtsideSpace = distanceInsideSpace * zoomOutsideSpace / zoomInsideSpace;

            return isZoomIn
                ? HeightRange.Minimum - distanceOtsideSpace
                : HeightRange.Maximum + distanceOtsideSpace;
        }

        #endregion

        /// <summary> Creates tile for given quadkey. </summary>
        protected Tile CreateTile(QuadKey quadKey, GameObject parent)
        {
            return new Tile(quadKey, _stylesheet, Projection, _elevationType, parent);
        }

        /// <summary> Loads given tile. </summary>
        protected void LoadTile(Tile tile)
        {
            _dataStore.OnNext(tile);
        }

        /// <summary> Calculates target zoom level for given distance. </summary>
        protected float CalculateZoom(float distance)
        {
            if (IsAboveMax)
                return LodRange.Maximum + 0.999f;

            if (IsBelowMin)
                return LodRange.Minimum;

            var lodRange = LodTree[distance].Single();
            return lodRange.Value + (lodRange.To - distance) / (lodRange.To - lodRange.From);
        }

        /// <summary> Unloads assets if necessary. </summary>
        /// <remarks> This method calls Resources.UnloadUnusedAssets which is expensive to do frequently. </remarks>
        protected void UnloadAssets(int tilesDisposed)
        {
            const int disposedTileThreshold = 20;

            _disposedTilesCounter += tilesDisposed;

            if (disposedTileThreshold > tilesDisposed)
            {
                _disposedTilesCounter = 0;
                Resources.UnloadUnusedAssets();
            }
        }
    }
}
