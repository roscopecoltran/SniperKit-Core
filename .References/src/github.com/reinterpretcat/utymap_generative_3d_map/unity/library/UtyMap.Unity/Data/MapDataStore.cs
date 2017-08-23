using System;
using System.Collections.Generic;
using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Data.Providers;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyRx;

namespace UtyMap.Unity.Data
{
    /// <summary> Defines behavior of class responsible of mapdata processing. </summary>
    public interface IMapDataStore : IObserver<Tile>, IObservable<MapData>, IObservable<Tile>
    {
        /// <summary> Adds mapdata to the specific dataStorage. </summary>
        /// <param name="dataStorage"> Storage type. </param>
        /// <param name="dataPath"> Path to mapdata. </param>
        /// <param name="stylesheet"> Stylesheet which to use during import. </param>
        /// <param name="levelOfDetails"> Which level of details to use. </param>
        /// <returns> Returns progress status. </returns>
        IObservable<int> Add(MapDataStorageType dataStorage, string dataPath, Stylesheet stylesheet, Range<int> levelOfDetails);

        /// <summary> Adds mapdata to the specific dataStorage. </summary>
        /// <param name="dataStorage"> Storage type. </param>
        /// <param name="dataPath"> Path to mapdata. </param>
        /// <param name="stylesheet"> Stylesheet which to use during import. </param>
        /// <param name="quadKey"> QuadKey to add. </param>
        /// <returns> Returns progress status. </returns>
        IObservable<int> Add(MapDataStorageType dataStorage, string dataPath, Stylesheet stylesheet, QuadKey quadKey);
    }

    /// <summary> Default implementation of map data store. </summary>
    internal class MapDataStore : IMapDataStore, IDisposable, IConfigurable
    {
        private readonly IMapDataProvider _mapDataProvider;
        private readonly IMapDataLibrary _mapDataLibrary;
        private MapDataStorageType _mapDataStorageType;

        private readonly List<IObserver<MapData>> _dataObservers = new List<IObserver<MapData>>();
        private readonly List<IObserver<Tile>> _tileObservers = new List<IObserver<Tile>>();

        [Dependency]
        public MapDataStore(IMapDataProvider mapDataProvider, IMapDataLibrary mapDataLibrary)
        {
            _mapDataProvider = mapDataProvider;
            _mapDataLibrary = mapDataLibrary;
            _mapDataProvider
                .ObserveOn(Scheduler.ThreadPool)
                .Subscribe(value =>
                {
                    // we have map data in store.
                    if (String.IsNullOrEmpty(value.Item2))
                        _mapDataLibrary.Get(value.Item1, _dataObservers);
                    else
                        Add(_mapDataStorageType, value.Item2, value.Item1.Stylesheet, value.Item1.QuadKey)
                            .Subscribe(progress => { }, 
                                       () => _mapDataLibrary.Get(value.Item1, _dataObservers));
                });
        }

        #region Interface implementations

        /// <inheritdoc />
        public IObservable<int> Add(MapDataStorageType dataStorageType, string dataPath, Stylesheet stylesheet, Range<int> levelOfDetails)
        {
            return _mapDataLibrary.Add(dataStorageType, dataPath, stylesheet, levelOfDetails);
        }

        /// <inheritdoc />
        public IObservable<int> Add(MapDataStorageType dataStorageType, string dataPath, Stylesheet stylesheet, QuadKey quadKey)
        {
            return _mapDataLibrary.Exists(quadKey)
                ? Observable.Return<int>(100)
                : _mapDataLibrary.Add(dataStorageType, dataPath, stylesheet, quadKey);
        }

        /// <inheritdoc />
        public virtual void OnCompleted()
        {
            _dataObservers.ForEach(o => o.OnCompleted());
            _tileObservers.ForEach(o => o.OnCompleted());
        }

        /// <inheritdoc />
        public virtual void OnError(Exception error)
        {
            _dataObservers.ForEach(o => o.OnError(error));
            _tileObservers.ForEach(o => o.OnError(error));
        }

        /// <inheritdoc />
        public void OnNext(Tile tile)
        {
            _mapDataProvider.OnNext(tile);
        }

        /// <summary> Subscribes on mesh/element data loaded events. </summary>
        public IDisposable Subscribe(IObserver<MapData> observer)
        {
            _dataObservers.Add(observer);
            return Disposable.Empty;
        }

        /// <summary> Subscribes on tile fully load event. </summary>
        public IDisposable Subscribe(IObserver<Tile> observer)
        {
            _tileObservers.Add(observer);
            return Disposable.Empty;
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _mapDataLibrary.Configure(configSection.GetString("data/index"));
            _mapDataStorageType = MapDataStorageType.Persistent;
        }

        /// <inheritdoc />
        public void Dispose()
        {
            _mapDataLibrary.Dispose();
        }

        #endregion
    }
}
