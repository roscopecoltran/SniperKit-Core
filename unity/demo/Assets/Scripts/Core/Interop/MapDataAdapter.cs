using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using Assets.Scripts.Core.Plugins;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Utils;
using UtyRx;
using Mesh = UtyMap.Unity.Mesh;

namespace Assets.Scripts.Core.Interop
{
    /// <summary> Adapts map tile data received from utymap API to the type used by the app. </summary>
    /// <remarks>
    ///     TODO refactor this class to reduce complexity and uglyness
    /// </remarks>
    internal static class MapDataAdapter
    {
        private static readonly Regex ElementNameRegex = new Regex("^(building):([0-9]*)");
        private const int VertexLimit = 64998;
        private const string TraceCategory = "mapdata.loader";
        
        /// <summary> Adapts mesh data received in raw form. </summary>
        public static void AdaptMesh(Tile tile, MaterialProvider materialProvider, IList<IObserver<MapData>> observers, ITrace trace,
            string name, double[] vertices, int[] triangles, int[] colors, double[] uvs, int[] uvMap)
        {
            Vector3[] worldPoints;
            Color[] unityColors;
            Vector2[] unityUvs;
            Vector2[] unityUvs2;
            Vector2[] unityUvs3;
            int textureIndex;

            // NOTE process terrain differently to emulate flat shading effect by avoiding 
            // triangles to share the same vertex. Remove "if" branch if you don't need it
            bool isCreated = name.Contains("terrain")
                ? BuildTerrainMesh(tile, materialProvider, name, vertices, triangles, colors, uvs, uvMap,
                    out worldPoints, out unityColors,  out textureIndex, out unityUvs, out unityUvs2, out unityUvs3)
                : BuildObjectMesh(tile, materialProvider, name, vertices, triangles, colors, uvs, uvMap,
                    out worldPoints, out unityColors, out textureIndex, out unityUvs, out unityUvs2, out unityUvs3);

            if (isCreated)
                BuildMesh(tile, observers, trace, name, worldPoints, triangles, unityColors, 
                    textureIndex, unityUvs, unityUvs2, unityUvs3);
        }

        /// <summary> Adapts element data received from utymap. </summary>
        public static void AdaptElement(Tile tile, MaterialProvider materialProvider, IList<IObserver<MapData>> observers, ITrace trace, 
            long id, double[] vertices, string[] tags, string[] styles)
        {
            int vertexCount = vertices.Length;
            var geometry = new GeoCoordinate[vertexCount / 3];
            var heights = new double[vertexCount / 3];
            for (int i = 0; i < vertexCount; i += 3)
            {
                geometry[i / 3] = new GeoCoordinate(vertices[i + 1], vertices[i]);
                heights[i / 3] = vertices[i + 2];
            }

            Element element = new Element(id, geometry, heights, ReadDict(tags), ReadDict(styles));
            NotifyObservers(new MapData(tile, new Union<Element, Mesh>(element)), observers);
        }

        #region Private members

        private static bool BuildTerrainMesh(Tile tile, MaterialProvider materialProvider, string name,
            double[] vertices, int[] triangles, int[] colors, double[] uvs, int[] uvMap,
            out Vector3[] worldPoints, out Color[] unityColors, out int textureIndex,
            out Vector2[] unityUvs, out Vector2[] unityUvs2, out Vector2[] unityUvs3)
        {
            worldPoints = new Vector3[triangles.Length];
            unityColors = new Color[triangles.Length];

            var atlasMapper = CreateTextureAtlasMapper(triangles.Length, uvs, uvMap, materialProvider);

            for (int i = 0; i < triangles.Length; ++i)
            {
                int vertIndex = triangles[i] * 3;
                worldPoints[i] = tile.Projection
                    .Project(new GeoCoordinate(vertices[vertIndex + 1], vertices[vertIndex]), vertices[vertIndex + 2]);

                unityColors[i] = ColorUtils.FromInt(colors[triangles[i]]);
                atlasMapper.SetUvs(i, triangles[i] * 2);
                triangles[i] = i;
            }

            unityUvs = atlasMapper.UnityUvs;
            unityUvs2 = atlasMapper.UnityUvs2;
            unityUvs3 = atlasMapper.UnityUvs3;
            textureIndex = atlasMapper.TextureIndex;

            return true;
        }

        private static bool BuildObjectMesh(Tile tile, MaterialProvider materialProvider, string name,
            double[] vertices, int[] triangles, int[] colors, double[] uvs, int[] uvMap,
            out Vector3[] worldPoints, out Color[] unityColors, out int textureIndex,
            out Vector2[] unityUvs, out Vector2[] unityUvs2, out Vector2[] unityUvs3)
        {
            long id;
            if (!ShouldLoad(tile, name, out id))
            {
                worldPoints = null;
                unityColors = null; ;
                unityUvs = null;
                unityUvs2 = null;
                unityUvs3 = null;
                textureIndex = 0;
                return false;
            }

            int uvCount = uvs.Length;
            int colorCount = colors.Length;

            worldPoints = new Vector3[vertices.Length / 3];
            for (int i = 0; i < vertices.Length; i += 3)
                worldPoints[i / 3] = tile.Projection
                    .Project(new GeoCoordinate(vertices[i + 1], vertices[i]), vertices[i + 2]);

            unityColors = new Color[colorCount];
            for (int i = 0; i < colorCount; ++i)
                unityColors[i] = ColorUtils.FromInt(colors[i]);
            
            if (uvCount > 0)
            {
                var textureMapper = CreateTextureAtlasMapper(uvCount / 2, uvs, uvMap, materialProvider);
                for (int i = 0; i < uvCount; i += 2)
                {
                    textureMapper.UnityUvs[i / 2] = new Vector2((float)uvs[i], (float)uvs[i + 1]);
                    textureMapper.SetUvs(i / 2, i);
                }

                unityUvs = textureMapper.UnityUvs;
                unityUvs2 = textureMapper.UnityUvs2;
                unityUvs3 = textureMapper.UnityUvs3;
                textureIndex = textureMapper.TextureIndex;
            }
            else
            {
                unityUvs = new Vector2[worldPoints.Length];
                unityUvs2 = null;
                unityUvs3 = null;
                textureIndex = 0;
            }

            tile.Register(id);

            return true;
        }

        private static Dictionary<string, string> ReadDict(string[] data)
        {
            var map = new Dictionary<string, string>(data.Length / 2);
            for (int i = 0; i < data.Length; i += 2)
                map.Add(data[i], data[i + 1]);
            return map;
        }

        private static bool ShouldLoad(Tile tile, string name, out long id)
        {
            var match = ElementNameRegex.Match(name);
            if (!match.Success)
            {
                id = 0;
                return true;
            }

            id = long.Parse(match.Groups[2].Value);
            return id == 0 || !tile.Has(id);
        }

        private static TextureAtlasMapper CreateTextureAtlasMapper(int triangleCount, double[] uvs, int[] uvMap, MaterialProvider materialProvider)
        {
            const int infoEntrySize = 8;
            var count = uvMap == null ? 0 : uvMap.Length;
            List<TextureAtlasInfo> infos = new List<TextureAtlasInfo>(count / infoEntrySize);
            for (int i = 0; i < count; )
            {
                var info = new TextureAtlasInfo();
                info.UvIndexRange = new Range<int>(!infos.Any() ? 0 : infos.Last().UvIndexRange.Maximum, uvMap[i++]);

                int textureIndex = uvMap[i++];
                info.TextureIndex = textureIndex;
                info.HasAtlas = materialProvider.HasAtlas(textureIndex);

                int atlasWidth = uvMap[i++];
                int atlasHeight = uvMap[i++];
                float x = uvMap[i++];
                float y = uvMap[i++];
                float width = uvMap[i++];
                float height = uvMap[i++];

                bool isEmpty = atlasWidth == 0 || atlasHeight == 0;
                info.TextureSize = new Vector2(isEmpty ? 0 : width / atlasWidth, isEmpty ? 0 : height / atlasHeight);
                info.TextureOffset = new Vector2(isEmpty ? 0 : x / atlasWidth, isEmpty ? 0 : y / atlasHeight);

                infos.Add(info);
            }

            return new TextureAtlasMapper(triangleCount, uvs, infos);
        }

        /// <summary> Builds mesh object and notifies observers. </summary>
        /// <remarks> Unity has vertex count limit and spliiting meshes here is quite expensive operation. </remarks>
        private static void BuildMesh(Tile tile, IList<IObserver<MapData>> observers, ITrace trace,
            string name, Vector3[] worldPoints, int[] triangles, Color[] unityColors, int textureIndex,
            Vector2[] unityUvs, Vector2[] unityUvs2, Vector2[] unityUvs3)
        {
            if (worldPoints.Length < VertexLimit)
            {
                Mesh mesh = new Mesh(name, textureIndex, worldPoints, triangles, unityColors, unityUvs, unityUvs2, unityUvs3);
                NotifyObservers(new MapData(tile, new Union<Element, Mesh>(mesh)), observers);
                return;
            }

            trace.Warn(TraceCategory, "Mesh '{0}' has more vertices than allowed by Unity: {1}. Will try to split..",
                   name, worldPoints.Length.ToString());
            if (worldPoints.Length != triangles.Length)
            {
                // TODO handle this case properly
                trace.Warn(TraceCategory, "Cannot split mesh {0}: vertecies count != triangles count", name);
                return;
            }

            int parts = (int) Math.Ceiling((float) worldPoints.Length / VertexLimit);
            for (int i = 0; i < parts; ++i)
            {
                var start = i * VertexLimit;
                var end = Math.Min(start + VertexLimit, worldPoints.Length);
                Mesh mesh = new Mesh(name + i, textureIndex, 
                    worldPoints.Skip(start).Take(end - start).ToArray(),
                    i == 0
                        ? triangles.Skip(start).Take(end - start).ToArray()
                        : triangles.Skip(start).Take(end - start).Select(tri => tri - start).ToArray(),
                    unityColors.Skip(start).Take(end - start).ToArray(),
                    unityUvs.Skip(start).Take(end - start).ToArray(),
                    unityUvs2 != null ? unityUvs2.Skip(start).Take(end - start).ToArray() : null,
                    unityUvs3 != null ? unityUvs3.Skip(start).Take(end - start).ToArray() : null);
                NotifyObservers(new MapData(tile, new Union<Element, Mesh>(mesh)), observers);
            }
        }

        private static void NotifyObservers(MapData mapData, IList<IObserver<MapData>> observers)
        {
            foreach (var observer in observers)
                observer.OnNext(mapData);
        }

        private static void NotifyObservers(Exception ex, IList<IObserver<MapData>> observers)
        {
            foreach (var observer in observers)
                observer.OnError(ex);
        }

        #endregion

        #region Nested class

        private class TextureAtlasMapper
        {
            public readonly Vector2[] UnityUvs;
            public readonly Vector2[] UnityUvs2;
            public readonly Vector2[] UnityUvs3;
            public readonly int TextureIndex;

            private readonly double[] _uvs;
            private readonly List<TextureAtlasInfo> _infos;

            public TextureAtlasMapper(int triangleCount, double[] uvs, List<TextureAtlasInfo> infos)
            {
                UnityUvs = new Vector2[triangleCount];
                UnityUvs2 = GetExtraUvs(triangleCount, infos);
                UnityUvs3 = GetExtraUvs(triangleCount, infos);
                TextureIndex = infos.Any() ? infos.First().TextureIndex : 0;

                _uvs = uvs;
                _infos = infos;
            }

            private static Vector2[] GetExtraUvs(int triangleCount, List<TextureAtlasInfo> infos)
            {
                return infos.Any() && infos.First().HasAtlas ? new Vector2[triangleCount] : null;
            }

            public void SetUvs(int resultIndex, int origIindex)
            {
                int begin = 0;
                int end = _infos.Count;

                while (begin < end)
                {
                    int middle = begin + (end - begin) / 2;
                    var info = _infos[middle];
                    if (info.UvIndexRange.Contains(origIindex))
                    {
                        UnityUvs[resultIndex] = new Vector2((float)_uvs[origIindex], (float)_uvs[origIindex + 1]);
                        if (info.HasAtlas)
                        {
                            UnityUvs2[resultIndex] = info.TextureSize;
                            UnityUvs3[resultIndex] = info.TextureOffset;
                        }
                        return;
                    }
                    if (info.UvIndexRange.Minimum > origIindex)
                        end = middle;
                    else
                        begin = middle + 1;
                }
            }
        }

        private struct TextureAtlasInfo
        {
            public int TextureIndex;
            public bool HasAtlas;
            public Range<int> UvIndexRange;
            public Vector2 TextureSize;
            public Vector2 TextureOffset;
        }

        #endregion
    }
}
