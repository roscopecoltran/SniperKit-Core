using System;
using UtyMap.Unity.Data;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Tests.Helpers;
using UtyMap.Unity.Utils;
using UtyRx;

namespace UtyMap.Unity.Tests
{
    /// <summary> This class is useful to debug/profile application independenly from Unity Editor. </summary>
    internal class Program
    {
        private const int LevelOfDetails = 16;

        private static void Main(string[] args)
        {
            // select main thread scheduler. it is different for Unity
            Scheduler.MainThread = Scheduler.CurrentThread;

            // get store and select theme represented by stylesheet
            var store = TestHelper.GetCompositionRoot(TestHelper.WorldZeroPoint)
                                  .GetService<IMapDataStore>();
            // create stylesheet from default mapcss
            var stylesheet = new Stylesheet(TestHelper.DefaultMapCss);

            // 1. add map data to store
            store.Add(MapDataStorageType.InMemory,
                      // file with map data
                      TestHelper.BerlinXmlData,
                      // mapcss stylesheet to control what is loaded
                      stylesheet,
                      // load data only for specific LOD (alternative API is available)
                      new Range<int>(LevelOfDetails, LevelOfDetails));
            
            // 2. start listening for processed data
            store
                // limiting speed of element processing
                .ThrottleFirst<MapData>(TimeSpan.FromMilliseconds(300))
                // move extensive processing away from main thread
                .SubscribeOn(Scheduler.ThreadPool)
                // but process results on main thread
                .ObserveOn(Scheduler.MainThread)
                // convert element or mesh into your specific representation
                .Subscribe(r=> r.Variant.Match(e => Console.WriteLine("Element: {0}", e.Id),
                                               m => Console.WriteLine("Mesh: {0}", m.Name)),
                           ex => Console.WriteLine("Error: {0}", ex));

            // 3. start loading of specific region
            store.OnNext(new Tile(
                // specify quadkey
                GeoUtils.CreateQuadKey(TestHelper.WorldZeroPoint, LevelOfDetails),
                // specify stylesheet which is used to create meshes
                stylesheet,
                // specify projection used by map data adapter
                new CartesianProjection(TestHelper.WorldZeroPoint),
                // specify elevation data type
                ElevationDataType.Flat));

            Console.ReadLine();
        }
    }
}