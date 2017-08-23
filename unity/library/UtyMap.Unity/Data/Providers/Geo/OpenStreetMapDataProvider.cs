using System;
using System.IO;
using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Utils;

namespace UtyMap.Unity.Data.Providers.Geo
{
    /// <summary> Downloads map data from openstreemap servers. </summary>
    internal class OpenStreetMapDataProvider : RemoteMapDataProvider
    {
        private string _importPath;
        private string _mapDataFormatExtension;
        private string _mapDataServerQuery;
        private string _mapDataServerUri;

        [Dependency]
        public OpenStreetMapDataProvider(IFileSystemService fileSystemService, 
                                         INetworkService networkService,
                                         ITrace trace)
            : base(fileSystemService, networkService, trace)
        {
        }

        /// <inheritdoc />
        public override void Configure(IConfigSection configSection)
        {
            _mapDataServerUri = configSection.GetString(@"data/osm/server", null);
            _mapDataServerQuery = configSection.GetString(@"data/osm/query", null);
            _mapDataFormatExtension = "." + configSection.GetString(@"data/osm/format", "xml");
            _importPath = configSection.GetString(@"data/import", null);
        }

        /// <inheritdoc />
        protected override string GetUri(QuadKey quadKey)
        {
            var padding = 0.001;
            var query = GeoUtils.QuadKeyToBoundingBox(quadKey);
            var queryString = String.Format(_mapDataServerQuery,
                query.MinPoint.Latitude - padding, query.MinPoint.Longitude - padding,
                query.MaxPoint.Latitude + padding, query.MaxPoint.Longitude + padding);
            return String.Format("{0}{1}", _mapDataServerUri, Uri.EscapeDataString(queryString));
        }

        /// <inheritdoc />
        protected override string GetFilePath(QuadKey quadKey)
        {
            return Path.Combine(_importPath, quadKey + _mapDataFormatExtension);
        }
    }
}
