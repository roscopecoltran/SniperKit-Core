using UnityEngine;

namespace UtyMap.Unity
{
    /// <summary> Represents mesh with all geometry needed. </summary>
    public class Mesh
    {
        public readonly string Name;
        public readonly Vector3[] Vertices;
        public readonly int[] Triangles;
        public readonly Color[] Colors;

        public readonly int TextureIndex;
        public readonly Vector2[] Uvs;
        public readonly Vector2[] Uvs2;
        public readonly Vector2[] Uvs3;

        public Mesh(string name, int textureIndex, Vector3[] vertices, int[] triangles, Color[] colors,
                    Vector2[] uvs, Vector2[] uvs2, Vector2[] uvs3)
        {
            Name = name;
            Vertices = vertices;
            Triangles = triangles;
            Colors = colors;

            TextureIndex = textureIndex;
            Uvs = uvs;
            Uvs2 = uvs2;
            Uvs3 = uvs3;
        }

    }
}
