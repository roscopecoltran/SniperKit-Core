using UtyMap.Unity.Infrastructure.Primitives;

namespace UtyMap.Unity
{
    /// <summary> Encapsulates map data variant which belongs to specific tile. </summary>
    public sealed class MapData
    {
        public readonly Tile Tile;
        public readonly Union<Element, Mesh> Variant;

        public MapData(Tile tile, Union<Element, Mesh> variant)
        {
            Tile = tile;
            Variant = variant;
        }
    }
}
