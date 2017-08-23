using System;
using System.Collections.Generic;
using System.IO;
using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;

namespace UtyMap.Unity.Data.Providers.Geo
{
    /// <summary> Downloads map data from mapzen servers. </summary>
    internal class MapzenMapDataProvider : RemoteMapDataProvider
    {
        /// <summary> Provides the way to control what to request from server. </summary>
        private static readonly Dictionary<int, int> LodMapping = new Dictionary<int, int>()
        {
            {2, 1}, {3, 3}, {4, 3}, {5, 4}, {6, 5}, {7, 6}, {8, 7}
        };

        private string _importPath;
        private string _mapDataServerUri;
        private string _mapDataFormatExtension;
        private string _mapDataLayers;
        private string _mapDataApiKey;
        
        [Dependency]
        public MapzenMapDataProvider(IFileSystemService fileSystemService, INetworkService networkService, ITrace trace) :
            base(fileSystemService, networkService, trace)
        {
        }

        /// <inheritdoc />
        public override void Configure(IConfigSection configSection)
        {
            _mapDataServerUri = configSection.GetString(@"data/mapzen/server", null);
            _mapDataFormatExtension = "." + configSection.GetString(@"data/mapzen/format", "json");
            _mapDataApiKey = configSection.GetString(@"data/mapzen/apikey", null);
            _mapDataLayers = configSection.GetString(@"data/mapzen/layers", null);

            _importPath = configSection.GetString(@"data/import", null);
        }

        /// <inheritdoc />
        protected override string GetUri(QuadKey quadKey)
        {
            var quadKeyToDownload = GetDownloadQuadKey(quadKey);

            return String.Format(_mapDataServerUri, _mapDataLayers, quadKeyToDownload.LevelOfDetail, 
                quadKeyToDownload.TileX, quadKeyToDownload.TileY, _mapDataApiKey);
        }

        /// <inheritdoc />
        protected override string GetFilePath(QuadKey quadKey)
        {
            return Path.Combine(_importPath, GetDownloadQuadKey(quadKey) + _mapDataFormatExtension);
        }

        /// <summary> Gets quadkey which should be downloaded from remote server. </summary>
        /// <remarks> Sometimes, mapzen returns more precise shapes then we need. </remarks>
        private QuadKey GetDownloadQuadKey(QuadKey quadKey)
        {
            if (!LodMapping.ContainsKey(quadKey.LevelOfDetail))
                return quadKey;

            int targetLod = LodMapping[quadKey.LevelOfDetail];
            var isParent = targetLod < quadKey.LevelOfDetail;
            var lod = quadKey.LevelOfDetail;
            var newQuadKey = quadKey;
            while (lod != targetLod)
            {
                var str = newQuadKey.ToString();
                newQuadKey = QuadKey.FromString(str.Substring(0, str.Length - 1));
                lod += (isParent ? -1 : 1);
            }

            return newQuadKey;
        }
    }
}
