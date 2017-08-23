using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using Assets.Scripts.Core.Plugins;
using UtyMap.Unity;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyRx;

namespace Assets.Scripts.Core.Interop
{
    /// <summary> Mono scripting backend specific implementation. </summary>
    partial class MapDataLibrary
    {
#if !ENABLE_IL2CPP
        private readonly ITrace _trace;

        /// <inheritdoc />
        public IObservable<int> Get(Tile tile, IList<IObserver<MapData>> observers)
        {
            TileHandler tileHandler = new TileHandler(tile, _materialProvider, observers, _trace);
            return Get(tile, tile.GetHashCode(), tileHandler.OnMeshBuiltHandler, 
                tileHandler.OnElementLoadedHandler, OnErrorHandler);
        }

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

        private static void OnCreateDirectory(string directory)
        {
            Directory.CreateDirectory(directory);
        }

        private static void OnErrorHandler(string message)
        {
            throw new InvalidOperationException(message);
        }

        private class TileHandler
        {
            private readonly Tile _tile;
            private readonly MaterialProvider _materialProvider;
            private readonly IList<IObserver<MapData>> _observers;
            private readonly ITrace _trace;

            public TileHandler(Tile tile, MaterialProvider materialProvider, IList<IObserver<MapData>> observers, ITrace trace)
            {
                _tile = tile;
                _materialProvider = materialProvider;
                _observers = observers;
                _trace = trace;
            }

            public void OnMeshBuiltHandler(int tag, string name, double[] vertices, int vertexCount,
                int[] triangles, int triangleCount, int[] colors, int colorCount,
                double[] uvs, int uvCount, int[] uvMap, int uvMapCount)
            {
                MapDataAdapter.AdaptMesh(_tile, _materialProvider, _observers, _trace, name, vertices, triangles, colors, uvs, uvMap);
            }

            public void OnElementLoadedHandler(int tag, long id, string[] tags, int tagCount,
                double[] vertices, int vertexCount, string[] styles, int styleCount)
            {
                MapDataAdapter.AdaptElement(_tile, _materialProvider, _observers, _trace, id, vertices, tags, styles);
            }
        }
     
        #endregion

#endif
    }
}
