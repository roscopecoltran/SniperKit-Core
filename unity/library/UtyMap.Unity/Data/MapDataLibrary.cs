using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using UnityEngine;
using UtyDepend;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Utils;
using UtyRx;

namespace UtyMap.Unity.Data
{
    /// <summary> Encapsulates low level API of underlying map data processing library. </summary>
    /// <remarks>
    ///     This interface provides the way to override default behavior of how map data is
    ///     added and received from native library. For example, il2cpp script backend requires 
    ///     to use different PInvoke mechanisms than mono.
    ///     Use <see cref="IMapDataStore"/> for any app specific logic.
    /// </remarks>
    public interface IMapDataLibrary : IDisposable
    {
        /// <summary> Configure utymap. Should be called before any core API usage. </summary>
        /// <param name="indexPath"> Path to index data. </param>
        void Configure(string indexPath);

        /// <summary> Enable mesh caching mechanism for speed up tile loading. </summary>
        void EnableCache();

        /// <summary> Disable mesh caching mechanism. </summary>
        void DisableCache();

        /// <summary> Checks whether there is data for given quadkey. </summary>
        /// <returns> True if there is data for given quadkey. </returns>
        bool Exists(QuadKey quadKey);

        /// <summary> Gets content of the tile notifying passed observers. </summary>
        /// <param name="tile"> Tile to load. </param>
        /// <param name="observers"> Observers to notify. </param>
        IObservable<int> Get(Tile tile, IList<IObserver<MapData>> observers);

        /// <summary>
        ///     Adds map data to data storage only to specific quadkey.
        ///     Supported formats: shapefile, osm xml, geo json, osm pbf.
        /// </summary>
        /// <param name="type"> Map data storage type. </param>
        /// <param name="path"> Path to file. </param>
        /// <param name="stylesheet"> Stylesheet path. </param>
        /// <param name="levelOfDetails"> Level of details. </param>
        IObservable<int> Add(MapDataStorageType type, string path, Stylesheet stylesheet, Range<int> levelOfDetails);

        /// <summary>
        ///     Adds map data to data storage only to specific quadkey.
        ///     Supported formats: shapefile, osm xml, geo json, osm pbf.
        /// </summary>
        /// <param name="type"> Map data storage type. </param>
        /// <param name="path"> Path to file. </param>
        /// <param name="stylesheet"> Stylesheet path. </param>
        /// <param name="quadKey"> QuadKey. </param>
        IObservable<int> Add(MapDataStorageType type, string path, Stylesheet stylesheet, QuadKey quadKey);

        /// <summary>
        ///     Adds element to data storage to specific level of details.
        /// </summary>
        /// <param name="type"> Map data storage type. </param>
        /// <param name="element"> Element to add. </param>
        /// <param name="stylesheet"> Stylesheet </param>
        /// <param name="levelOfDetails"> Level of detail range. </param>
        IObservable<int> Add(MapDataStorageType type, Element element, Stylesheet stylesheet, Range<int> levelOfDetails);

        /// <summary>
        ///      Gets elevation for specific coordinate and quadKey.
        /// </summary>
        /// <param name="elevationDataType"></param>
        /// <param name="quadKey"></param>
        /// <param name="coordinate"></param>
        double GetElevation(ElevationDataType elevationDataType, QuadKey quadKey, GeoCoordinate coordinate);
    }

    /// <summary> Default implementation. </summary>
    internal class MapDataLibrary : IMapDataLibrary
    {
        private const string TraceCategory = "library";
        private const string InMemoryStoreKey = "InMemory";
        private const string PersistentStoreKey = "Persistent";

        private readonly object __lockObj = new object();
        private readonly IPathResolver _pathResolver;
        private readonly ITrace _trace;
        private volatile bool _isConfigured;

        private HashSet<string> _stylePaths = new HashSet<string>();

        [Dependency]
        public MapDataLibrary(IPathResolver pathResolver, ITrace trace)
        {
            _pathResolver = pathResolver;
            _trace = trace;
        }

        /// <inheritdoc />
        public IObservable<int> Get(Tile tile, IList<IObserver<MapData>> observers)
        {
            var tileHandler = new TileHandler(tile, observers);
            return Get(tile, tile.GetHashCode(), tileHandler.OnMeshBuiltHandler, tileHandler.OnElementLoadedHandler, OnErrorHandler);
        }

        /// <inheritdoc />
        public void Configure(string indexPath)
        {
            lock (__lockObj)
            {
                indexPath = _pathResolver.Resolve(indexPath);

                _trace.Debug(TraceCategory, "Configure with {0}", indexPath);
                // NOTE this directories should be created in advance (and some others..)
                if (!Directory.Exists(indexPath))
                    throw new DirectoryNotFoundException(String.Format("Cannot find {0}", indexPath));

                if (_isConfigured)
                    return;

                // create directory for downloaded raw map data.
                CreateDirectory(Path.Combine(indexPath, "import"));

                configure(indexPath, OnErrorHandler);
                // NOTE actually, it is possible to have multiple in-memory and persistent 
                // storages at the same time.
                registerInMemoryStore(InMemoryStoreKey);
                registerPersistentStore(PersistentStoreKey, indexPath, CreateDirectory);
                
                _isConfigured = true;
            }
        }

        /// <inheritdoc />
        public void EnableCache()
        {
            enableMeshCache(1);
        }

        /// <inheritdoc />
        public void DisableCache()
        {
            enableMeshCache(0);
        }

        /// <inheritdoc />
        public bool Exists(QuadKey quadKey)
        {
            return hasData(quadKey.TileX, quadKey.TileY, quadKey.LevelOfDetail);
        }

        /// <inheritdoc />
        public IObservable<int> Add(MapDataStorageType type, string path, Stylesheet stylesheet, Range<int> levelOfDetails)
        {
            var dataPath = _pathResolver.Resolve(path);
            var stylePath = RegisterStylesheet(stylesheet);
            _trace.Debug(TraceCategory, "Add data from {0} to {1} storage", dataPath, type.ToString());
            lock (__lockObj)
            {
                addToStoreInRange(GetStoreKey(type), stylePath, dataPath, levelOfDetails.Minimum,
                    levelOfDetails.Maximum, OnErrorHandler);
            }
            return Observable.Return<int>(100);
        }

        /// <inheritdoc />
        public IObservable<int> Add(MapDataStorageType type, string path, Stylesheet stylesheet, QuadKey quadKey)
        {
            var dataPath = _pathResolver.Resolve(path);
            var stylePath = RegisterStylesheet(stylesheet);
            _trace.Debug(TraceCategory, "Add data from {0} to {1} storage", dataPath, type.ToString());
            lock (__lockObj)
            {
                addToStoreInQuadKey(GetStoreKey(type), stylePath, dataPath, quadKey.TileX, quadKey.TileY,
                    quadKey.LevelOfDetail, OnErrorHandler);
            }
            return Observable.Return<int>(100);
        }

        /// <inheritdoc />
        public IObservable<int> Add(MapDataStorageType type, Element element, Stylesheet stylesheet, Range<int> levelOfDetails)
        {
            _trace.Debug(TraceCategory, "Add element to {0} storage", type.ToString());
            double[] coordinates = new double[element.Geometry.Length * 2];
            for (int i = 0; i < element.Geometry.Length; ++i)
            {
                coordinates[i * 2] = element.Geometry[i].Latitude;
                coordinates[i * 2 + 1] = element.Geometry[i].Longitude;
            }

            string[] tags = new string[element.Tags.Count * 2];
            var tagKeys = element.Tags.Keys.ToArray();
            for (int i = 0; i < tagKeys.Length; ++i)
            {
                tags[i * 2] = tagKeys[i];
                tags[i * 2 + 1] = element.Tags[tagKeys[i]];
            }

            var stylePath = RegisterStylesheet(stylesheet);

            lock (__lockObj)
            {
                addToStoreElement(GetStoreKey(type), stylePath, element.Id,
                    coordinates, coordinates.Length,
                    tags, tags.Length,
                    levelOfDetails.Minimum, levelOfDetails.Maximum, OnErrorHandler);
            }
            return Observable.Return<int>(100);
        }

        /// <inheritdoc />
        public double GetElevation(ElevationDataType elevationDataType, QuadKey quadKey, GeoCoordinate coordinate)
        {
            return getElevation(quadKey.TileX, quadKey.TileY, quadKey.LevelOfDetail,
                (int)elevationDataType, coordinate.Latitude, coordinate.Longitude);
        }

        /// <inheritdoc />
        public void Dispose()
        {
        }

        #region Private members

        private IObservable<int> Get(Tile tile, int tag, OnMeshBuilt meshBuiltHandler, OnElementLoaded elementLoadedHandler, OnError errorHandler)
        {
            _trace.Debug(TraceCategory, "Get tile {0}", tile.ToString());
            var stylePath = RegisterStylesheet(tile.Stylesheet);
            var quadKey = tile.QuadKey;
            var cancelTokenHandle = GCHandle.Alloc(tile.CancelationToken, GCHandleType.Pinned);
            loadQuadKey(tag, stylePath,
                quadKey.TileX, quadKey.TileY, quadKey.LevelOfDetail, (int)tile.ElevationType,
                meshBuiltHandler, elementLoadedHandler, errorHandler,
                cancelTokenHandle.AddrOfPinnedObject());
            cancelTokenHandle.Free();
            return Observable.Return(100);
        }

        private static string GetStoreKey(MapDataStorageType dataStorageType)
        {
            return dataStorageType == MapDataStorageType.InMemory ? InMemoryStoreKey : PersistentStoreKey;
        }

        private static void CreateDirectory(string directory)
        {
            Directory.CreateDirectory(directory);
        }

        private string RegisterStylesheet(Stylesheet stylesheet)
        {
            var stylePath = _pathResolver.Resolve(stylesheet.Path);

            if (_stylePaths.Contains(stylePath))
                return stylePath;

            _stylePaths.Add(stylePath);
            registerStylesheet(stylePath, CreateDirectory);

            return stylePath;
        }

        #endregion

        #region Delegates

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void OnNewDirectory([In] string directory);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void OnMeshBuilt(int tag, [In] string name,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] [In] double[] vertices, [In] int vertexCount,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 5)] [In] int[] triangles, [In] int triangleCount,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 7)] [In] int[] colors, [In] int colorCount,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 9)] [In] double[] uvs, [In] int uvCount,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 11)] [In] int[] uvMap, [In] int uvMapCount);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void OnElementLoaded(int tag, [In] long id,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] [In] string[] tags, [In] int tagCount,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 5)] [In] double[] vertices, [In] int vertexCount,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 7)] [In] string[] styles, [In] int styleCount);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void OnError([In] string message);

        #endregion

        #region Callbacks

        private class TileHandler
        {
            private readonly Tile _tile;
            private readonly IList<IObserver<MapData>> _observers;

            public TileHandler(Tile tile, IList<IObserver<MapData>> observers)
            {
                _tile = tile;
                _observers = observers;
            }

            public void OnMeshBuiltHandler(int tag, string name, double[] vertices, int vertexCount,
                int[] triangles, int triangleCount, int[] colors, int colorCount,
                double[] uvs, int uvCount, int[] uvMap, int uvMapCount)
            {
                var worldPoints = new Vector3[vertices.Length / 3];
                for (int i = 0; i < vertices.Length; i += 3)
                    worldPoints[i / 3] = _tile.Projection
                        .Project(new GeoCoordinate(vertices[i + 1], vertices[i]), vertices[i + 2]);

                var unityColors = new Color[colorCount];
                for (int i = 0; i < colorCount; ++i)
                    unityColors[i] = ColorUtils.FromInt(colors[i]);

                var unityUvs = new Vector2[triangleCount];
                var unityUvs2 = new Vector2[triangleCount];
                var unityUvs3 = new Vector2[triangleCount];

                Mesh mesh = new Mesh(name, 0, worldPoints, triangles, unityColors, unityUvs, unityUvs2, unityUvs3);
                NotifyObservers(new MapData(_tile, new Union<Element, Mesh>(mesh)));
            }

            public void OnElementLoadedHandler(int tag, long id, string[] tags, int tagCount,
                double[] vertices, int vertexCount, string[] styles, int styleCount)
            {
                var geometry = new GeoCoordinate[vertexCount / 3];
                var heights = new double[vertexCount / 3];
                for (int i = 0; i < vertexCount; i += 3)
                {
                    geometry[i / 3] = new GeoCoordinate(vertices[i + 1], vertices[i]);
                    heights[i / 3] = vertices[i + 2];
                }

                Element element = new Element(id, geometry, heights, ReadDict(tags), ReadDict(styles));
                NotifyObservers(new MapData(_tile, new Union<Element, Mesh>(element)));
            }

            private static Dictionary<string, string> ReadDict(string[] data)
            {
                var map = new Dictionary<string, string>(data.Length / 2);
                for (int i = 0; i < data.Length; i += 2)
                    map.Add(data[i], data[i + 1]);
                return map;
            }

            private void NotifyObservers(MapData mapData)
            {
                foreach (var observer in _observers)
                    observer.OnNext(mapData);
            }
        }

        private static void OnErrorHandler(string message)
        {
            throw new InvalidOperationException(message);
        }

        #endregion

        #region PInvoke import

        [DllImport("UtyMap.Shared")]
        private static extern void configure(string stringPath, OnError errorHandler);

        [DllImport("UtyMap.Shared")]
        private static extern void enableMeshCache(int enabled);

        [DllImport("UtyMap.Shared")]
        private static extern void registerStylesheet(string path, OnNewDirectory directoryHandler);

        [DllImport("UtyMap.Shared")]
        private static extern void registerInMemoryStore(string key);

        [DllImport("UtyMap.Shared")]
        private static extern void registerPersistentStore(string key, string path, OnNewDirectory directoryHandler);

        [DllImport("UtyMap.Shared")]
        private static extern void addToStoreInRange(string key, string stylePath, string path, int startLod, int endLod, OnError errorHandler);

        [DllImport("UtyMap.Shared")]
        private static extern void addToStoreInQuadKey(string key, string stylePath, string path, int tileX, int tileY, int lod, OnError errorHandler);

        [DllImport("UtyMap.Shared")]
        private static extern void addToStoreElement(string key, string stylePath, long id, double[] vertices, int vertexLength,
            string[] tags, int tagLength, int startLod, int endLod, OnError errorHandler);

        [DllImport("UtyMap.Shared")]
        private static extern bool hasData(int tileX, int tileY, int levelOfDetails);

        [DllImport("UtyMap.Shared")]
        private static extern double getElevation(int tileX, int tileY, int levelOfDetails, int eleDataType, double latitude, double longitude);

        [DllImport("UtyMap.Shared")]
        private static extern void loadQuadKey(int tag, string stylePath, int tileX, int tileY, int levelOfDetails, int eleDataType,
            OnMeshBuilt meshBuiltHandler, OnElementLoaded elementLoadedHandler, OnError errorHandler, IntPtr cancelToken);

        [DllImport("UtyMap.Shared")]
        private static extern void cleanup();

        #endregion
    }
}
