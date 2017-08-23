using UtyDepend.Config;

namespace UtyMap.Unity.Infrastructure.Config
{
    /// <summary> Represents a builder responsible for application configuration creation. </summary>
    public class ConfigBuilder
    {
        private readonly CodeConfigSection _configSection = new CodeConfigSection();

        /// <summary> Adds value to configuration with path provided. </summary>
        /// <remarks> Use this method to extend application with your custom settings. </remarks>
        /// <typeparam name="T">Type of value.</typeparam>
        /// <param name="path">Path.</param>
        /// <param name="value">Value.</param>
        public void Add<T>(string path, T value)
        {
            _configSection.Add(path, value);
        }

        /// <summary> Builds application specific configuration. </summary>
        public IConfigSection Build()
        {
            return _configSection;
        }

        #region Application specific

        /// <summary> Sets settings to get elevation data from remote server. </summary>
        public ConfigBuilder SetSrtmEleData(string url, string schema)
        {
            Add<string>("data/srtm/server", url);
            Add<string>("data/srtm/schema", schema);
            return this;
        }

        /// <summary> Sets geocoding server's url. </summary>
        public ConfigBuilder SetGeocodingServer(string url)
        {
            Add<string>("geocoding", url);
            return this;
        }

        /// <summary> Sets local path to elevation data. </summary>
        public ConfigBuilder SetLocalElevationData(string path)
        {
            Add<string>("data/elevation/local", path);
            return this;
        }

        /// <summary> Sets index path. </summary>
        public ConfigBuilder SetIndex(string path)
        {
            Add<string>("data/index", path);
            return this;
        }

        /// <summary> Sets osm map data provider. </summary>
        public ConfigBuilder SetOsmMapData(string url, string schema, string format)
        {
            Add<string>("data/osm/server", url);
            Add<string>("data/osm/query", schema);
            Add<string>("data/osm/format", format);
            return this;
        }

        /// <summary> Sets mapzen map data provider. </summary>
        public ConfigBuilder SetMapzenMapData(string url, string layers, string format, string apiKey)
        {
            Add<string>("data/mapzen/server", url);
            Add<string>("data/mapzen/layers", layers);
            Add<string>("data/mapzen/format", format);
            Add<string>("data/mapzen/apikey", apiKey);
            return this;
        }

        public ConfigBuilder SetMapzenEleData(string url, int gridSize, string format, string apiKey)
        {
            Add<string>("data/mapzen/ele_server", url);
            Add<int>("data/mapzen/ele_grid", gridSize);
            Add<string>("data/mapzen/ele_format", format);
            Add<string>("data/mapzen/api_key", apiKey);
            
            return this;
        }

        /// <summary> Sets import data path where raw map data is stored. </summary>
        public ConfigBuilder SetImport(string import)
        {
            Add<string>("data/import", import);
            return this;
        }

        #endregion

        #region Default instance

        /// <summary> Gets ConfigBuilder with default settings. </summary>
        /// <remarks> You can call methods to override settings with custom ones. </remarks>
        public static ConfigBuilder GetDefault()
        {
            return new ConfigBuilder()
                .SetLocalElevationData("index/data")
                .SetSrtmEleData("http://dds.cr.usgs.gov/srtm/version2_1/SRTM3", "config/srtm.schema.txt")
                .SetOsmMapData("http://overpass-api.de/api/interpreter?data=", "(node({0},{1},{2},{3}); <; >;);out body;", "xml")
                //.SetOsmMapData("http://api.openstreetmap.org/api/0.6/map?bbox=", "{1},{0},{3},{2}", "xml")
                .SetMapzenMapData("http://tile.mapzen.com/mapzen/vector/v1/{0}/{1}/{2}/{3}.json?api_key={4}", "all", "json", "mapzen-D92KwAq")
                .SetMapzenEleData("http://elevation.mapzen.com/height?json={0}&api_key={1}", 4, "ele", "mapzen-D92KwAq")
                .SetImport("index/import")
                .SetGeocodingServer("http://nominatim.openstreetmap.org/search?");
        }

        #endregion
    }
}
