using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UtyDepend;
using UtyMap.Unity;
using Mesh = UtyMap.Unity.Mesh;

namespace Assets.Scripts.Core.Plugins
{
    /// <summary> Responsible for building Unity game objects from meshes and elements. </summary>
    internal class GameObjectBuilder
    {
        private readonly IList<IElementBuilder> _elementBuilders;
        private readonly MaterialProvider _materialProvider;

        [Dependency]
        public GameObjectBuilder(MaterialProvider materialProvider, IEnumerable<IElementBuilder> elementBuilders)
        {
            _materialProvider = materialProvider;
            _elementBuilders = elementBuilders.ToList();
        }

        /// <inheritdoc />
        public void BuildFromElement(Tile tile, Element element)
        {
            foreach (var builder in _elementBuilders)
            {
                if (!element.Styles["builder"].Contains(builder.Name))
                    continue;

                var gameObject = builder.Build(tile, element);
                if (gameObject.transform.parent == null)
                    gameObject.transform.parent = tile.GameObject.transform;
            }
        }

        /// <inheritdoc />
        public void BuildFromMesh(Tile tile, Mesh mesh)
        {
            var gameObject = new GameObject(mesh.Name);

            var uMesh = new UnityEngine.Mesh();
            uMesh.vertices = mesh.Vertices;
            uMesh.triangles = mesh.Triangles;
            uMesh.colors = mesh.Colors;
            uMesh.uv = mesh.Uvs;
            uMesh.uv2 = mesh.Uvs2;
            uMesh.uv3 = mesh.Uvs3;

            uMesh.RecalculateNormals();

            gameObject.isStatic = true;
            gameObject.AddComponent<MeshFilter>().mesh = uMesh;
            gameObject.AddComponent<MeshRenderer>().sharedMaterial = _materialProvider.GetSharedMaterial(mesh.TextureIndex);
            gameObject.transform.parent = tile.GameObject.transform;
        }
    }
}
