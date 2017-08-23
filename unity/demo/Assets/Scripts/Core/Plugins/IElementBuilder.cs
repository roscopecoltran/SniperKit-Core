using UnityEngine;
using UtyMap.Unity;

namespace Assets.Scripts.Core.Plugins
{
    /// <summary> Provides the way to build custom representation of map data. </summary>
    public interface IElementBuilder
    {
        /// <summary> Unique name. </summary>
        string Name { get; }

        /// <summary> Builds gameobject from Element for given tile. </summary>
        GameObject Build(Tile tile, Element element);
    }
}