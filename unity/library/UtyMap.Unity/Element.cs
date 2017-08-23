using System.Collections.Generic;

namespace UtyMap.Unity
{
    /// <summary> OSM specific data reprsentation. </summary>
    public class Element
    {
        public readonly long Id;
        public readonly GeoCoordinate[] Geometry;
        public readonly double[] Heights;
        public readonly Dictionary<string, string> Tags;
        public readonly Dictionary<string, string> Styles;

        public Element(long id, GeoCoordinate[] geometry, double[] heights, Dictionary<string, string> tags, Dictionary<string, string> styles)
        {
            Id = id;
            Geometry = geometry;
            Heights = heights;
            Tags = tags;
            Styles = styles;
        }
    }
}
