using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using Assets.Scripts.Core.Plugins;
using UtyDepend;
using UtyMap.Unity;
using UtyMap.Unity.Data;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyRx;

namespace Assets.Scripts.Core.Interop
{
    internal sealed partial class MapDataLibrary : IMapDataLibrary
    {
        private const string TraceCategory = "library";
        private const string InMemoryStoreKey = "InMemory";
        private const string PersistentStoreKey = "Persistent";

        private static readonly object __lockObj = new object();
        private volatile bool _isConfigured;
        private readonly MaterialProvider _materialProvider;
        private readonly IPathResolver _pathResolver;

        private HashSet<string> _stylePaths = new HashSet<string>();

        [Dependency]
        public MapDataLibrary(MaterialProvider materialProvider, IPathResolver pathResolver, ITrace trace)
        {
            _materialProvider = materialProvider;
            _pathResolver = pathResolver;
            _trace = trace;
        }

        /// <inheritdoc />
        public void Configure(string indexPath)
        {
            lock (__lockObj)
            {
                indexPath = _pathResolver.Resolve(indexPath);

                _trace.Debug(TraceCategory, "Configure with {0}", indexPath);
                
                if (!Directory.Exists(indexPath))
                    throw new DirectoryNotFoundException(String.Format("Cannot find {0}", indexPath));

                if (_isConfigured)
                    return;

                configure(indexPath, OnErrorHandler);
                // NOTE actually, it is possible to have multiple in-memory and persistent 
                // storages at the same time.
                registerInMemoryStore(InMemoryStoreKey);
                registerPersistentStore(PersistentStoreKey, indexPath, OnCreateDirectory);
                // NOTE Enable mesh caching mechanism for speed up tile loading.
                enableMeshCache(1);
                
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

        private string RegisterStylesheet(Stylesheet stylesheet)
        {
            var stylePath = _pathResolver.Resolve(stylesheet.Path);

            if (_stylePaths.Contains(stylePath))
                return stylePath;

            _stylePaths.Add(stylePath);
            registerStylesheet(stylePath, OnCreateDirectory);

            return stylePath;
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
