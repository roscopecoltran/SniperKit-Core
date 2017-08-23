using System.Collections.Generic;
using UnityEngine;

namespace Assets.Scripts.Core.Plugins
{
    /// <summary> Provides unity materials.</summary>
    internal class MaterialProvider
    {
        private readonly Dictionary<string, Material> _sharedMaterials = new Dictionary<string, Material>();
        private readonly List<MaterialDescription> _descriptions = new List<MaterialDescription>()
        {
            new MaterialDescription(@"Materials/TextureColored", false),
            new MaterialDescription(@"Materials/AtlasColored", true)
        };

        /// <summary> Checks whether texture is atlas. </summary>
        public bool HasAtlas(int textureIndex)
        {
            return GetDescriptionByIndex(textureIndex).HasAtlas;
        }

        /// <summary> Gets shared material by texture index. </summary>
        /// <remarks> Should be called from UI thread only. </remarks>
        public Material GetSharedMaterial(int textureIndex)
        {
            return GetSharedMaterial(GetDescriptionByIndex(textureIndex).Path);
        }

        /// <summary> Gets shared material by path. </summary>
        /// <remarks> Should be called from UI thread only. </remarks>
        public Material GetSharedMaterial(string path)
        {
            if (!_sharedMaterials.ContainsKey(path))
                _sharedMaterials[path] = Resources.Load<Material>(path);

            return _sharedMaterials[path];
        }

        private MaterialDescription GetDescriptionByIndex(int textureIndex)
        {
            return _descriptions[textureIndex % _descriptions.Count];
        }

        private class MaterialDescription
        {
            public readonly string Path;
            public readonly bool HasAtlas;

            public MaterialDescription(string path, bool hasAtlas)
            {
                Path = path;
                HasAtlas = hasAtlas;
            }
        }
    }
}
