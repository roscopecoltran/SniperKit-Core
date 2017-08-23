using System;
using Assets.Scripts.Core.Interop;
using Assets.Scripts.Core.Plugins;
using Assets.Scripts.Environment;
using Assets.Scripts.Environment.Reactive;
using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity;
using UtyMap.Unity.Data;
using UtyMap.Unity.Infrastructure.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyRx;
using Component = UtyDepend.Component;

namespace Assets.Scripts.Core
{
    /// <summary> Provides the way to initialize utymap library. </summary>
    internal static class InitTask
    {
        /// <summary> Run library initialization logic. </summary>
        public static CompositionRoot Run(Action<IContainer, IConfigSection> action)
        {
            const string fatalCategoryName = "Fatal";

            // create trace for logging and set its level
            var trace = new UnityLogTrace();
            trace.Level = DefaultTrace.TraceLevel.Debug;

            // utymap requires some files/directories to be precreated.
            InstallationApi.EnsureFileHierarchy(trace);

            // setup RX configuration.
            UnityScheduler.SetDefaultForUnity();

            // subscribe to unhandled exceptions in RX
            MainThreadDispatcher.RegisterUnhandledExceptionCallback(ex => trace.Error(fatalCategoryName, ex, "Unhandled exception"));

            try
            {
                var compositionRoot = BuildCompositionRoot(action, trace);
                SubscribeOnMapData(compositionRoot, trace);
                return compositionRoot;
            }
            catch (Exception ex)
            {
                trace.Error(fatalCategoryName, ex, "Cannot setup object graph.");
                throw;
            }
        }

        /// <summary> Builds instance responsible for composing object graph. </summary>
        private static CompositionRoot BuildCompositionRoot(Action<IContainer, IConfigSection> action, ITrace trace)
        {
            // create entry point for library functionallity using default configuration overriding some properties
            return new CompositionRoot(new Container(), ConfigBuilder.GetDefault().SetIndex("index/").Build())
                // override library's default services with demo specific implementations
                .RegisterAction((container, config) =>
                {
                    container
                        .RegisterInstance<ITrace>(trace)
                        .Register(Component.For<IPathResolver>().Use<UnityPathResolver>())
                        .Register(Component.For<INetworkService>().Use<UnityNetworkService>())
                        .Register(Component.For<IMapDataLibrary>().Use<MapDataLibrary>());
                })
                // override with scene specific implementations
                .RegisterAction(action)
                // setup object graph
                .Setup();
        }

        /// <summary> Starts listening for mapdata from core library to convert it into unity game objects. </summary>
        private static void SubscribeOnMapData(CompositionRoot compositionRoot, ITrace trace)
        {
            const string traceCategory = "mapdata";
            var modelBuilder = compositionRoot.GetService<GameObjectBuilder>();
            compositionRoot.GetService<IMapDataStore>()
               .SubscribeOn<MapData>(Scheduler.ThreadPool)
               .ObserveOn(Scheduler.MainThread)
               .Where(r => !r.Tile.IsDisposed)
               .Subscribe(r => r.Variant.Match(
                               e => modelBuilder.BuildFromElement(r.Tile, e),
                               m => modelBuilder.BuildFromMesh(r.Tile, m)),
                          ex => trace.Error(traceCategory, ex, "cannot process mapdata."),
                          () => trace.Warn(traceCategory, "stop listening mapdata."));
        }
    }
}
