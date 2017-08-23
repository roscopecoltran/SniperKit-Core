using System;
using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Data.Providers.Elevation;
using UtyMap.Unity.Data.Providers.Geo;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyRx;

namespace UtyMap.Unity.Data.Providers
{
    /// <summary> 
    ///     Aggregates different map data providers and decides which one to use for specific tiles.
    /// </summary>
    internal sealed class AggregateMapDataProvider : MapDataProvider
    {
        private readonly IMapDataProvider _eleProvider;
        private readonly IMapDataProvider _dataProvider;
       
        [Dependency]
        public AggregateMapDataProvider(IFileSystemService fileSystemService, INetworkService networkService, 
            IMapDataLibrary mapDataLibrary, ITrace trace)
        {
            _eleProvider = new ElevationProvider(
                new MapzenElevationDataProvider(fileSystemService, networkService, trace),
                new SrtmElevationDataProvider(fileSystemService, networkService, trace));

            _dataProvider = new DataProvider(mapDataLibrary,
                new OpenStreetMapDataProvider(fileSystemService, networkService, trace),
                new MapzenMapDataProvider(fileSystemService, networkService, trace));
        }

        /// <inheritdoc />
        public override void OnNext(Tile value)
        {
            _eleProvider.OnNext(value);
        }

        /// <inheritdoc />
        public override void Configure(IConfigSection configSection)
        {
            _eleProvider.Configure(configSection);
            _eleProvider.Subscribe(t => _dataProvider.OnNext(t.Item1));

            _dataProvider.Configure(configSection);
            _dataProvider.Subscribe(Notify);
        }

        #region Nested classes

        /// <summary> Encapsulates elevation processing. </summary>
        private class ElevationProvider : MapDataProvider
        {          
            private readonly IMapDataProvider _mapzenEleProvider;
            private readonly IMapDataProvider _srtmEleProvider;

            public ElevationProvider(IMapDataProvider mapzenEleProvider, IMapDataProvider srtmEleProvider)
            {
                _mapzenEleProvider = mapzenEleProvider;
                _srtmEleProvider = srtmEleProvider;
            }

            /// <inheritdoc />
            public override void OnNext(Tile value)
            {
                if (value.ElevationType == ElevationDataType.Flat)
                {
                    Notify(new Tuple<Tile, string>(value, ""));
                    return;
                }
                
                if (value.ElevationType == ElevationDataType.Grid)
                    _mapzenEleProvider.OnNext(value);
                else
                    _srtmEleProvider.OnNext(value);
            }

            /// <inheritdoc />
            public override void Configure(IConfigSection configSection)
            {
                _mapzenEleProvider.Configure(configSection);
                _mapzenEleProvider.Subscribe(Notify);

                _srtmEleProvider.Configure(configSection);
                _srtmEleProvider.Subscribe(Notify);
            }
        }

        /// <summary> Encapsulates map data processing. </summary>
        class DataProvider : MapDataProvider
        {
            private readonly Range<int> OsmTileRange = new Range<int>(16, 16);

            private readonly IMapDataProvider _osmMapDataProvider;
            private readonly IMapDataProvider _mapzenMapDataProvider;
            private readonly IMapDataLibrary _mapDataLibrary;

            public DataProvider(IMapDataLibrary mapDataLibrary,
                IMapDataProvider osmMapDataProvider, IMapDataProvider mapzenMapDataProvider)
            {
                _osmMapDataProvider = osmMapDataProvider;
                _mapzenMapDataProvider = mapzenMapDataProvider;
                _mapDataLibrary = mapDataLibrary;
            }

            /// <inheritdoc />
            public override void OnNext(Tile value)
            {
                if (_mapDataLibrary.Exists(value.QuadKey))
                {
                    Notify(new Tuple<Tile, string>(value, ""));
                    return;
                }

                if (OsmTileRange.Contains(value.QuadKey.LevelOfDetail))
                    _osmMapDataProvider.OnNext(value);
                else
                    _mapzenMapDataProvider.OnNext(value);
            }

            /// <inheritdoc />
            public override void Configure(IConfigSection configSection)
            {
                _osmMapDataProvider.Configure(configSection);
                _osmMapDataProvider.Subscribe(Notify);

                _mapzenMapDataProvider.Configure(configSection);
                _mapzenMapDataProvider.Subscribe(Notify);
            }
        }

        #endregion
    }
}
