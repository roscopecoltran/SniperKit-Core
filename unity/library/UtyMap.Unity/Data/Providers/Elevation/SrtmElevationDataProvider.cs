using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UtyDepend.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.IO.Compression;
using UtyMap.Unity.Utils;

namespace UtyMap.Unity.Data.Providers.Elevation
{
    /// <summary> Downloads SRTM data from NASA server. </summary>
    internal class SrtmElevationDataProvider : RemoteMapDataProvider
    {
        private const string TraceCategory = "mapdata.srtm";
        private readonly object _lock = new object();

        private readonly IFileSystemService _fileSystemService;
        private readonly INetworkService _networkService;
        private readonly ITrace _trace;
        private string _server;
        private string _schemaPath;
        private string _elePath;

        private static readonly Dictionary<int, string> ContinentMap = new Dictionary<int, string>
        {
            {0, "Eurasia"},
            {1, "South_America"},
            {2, "Africa"},
            {3, "North_America"},
            {4, "Australia"},
            {5, "Islands"},
        };      

        /// <summary> Creates instance of <see cref="SrtmElevationDataProvider"/>. </summary>
        public SrtmElevationDataProvider(IFileSystemService fileSystemService, INetworkService networkService, ITrace trace) :
            base(fileSystemService, networkService, trace)
        {
            _fileSystemService = fileSystemService;
            _networkService = networkService;
            _trace = trace;
        }

        #region Public methods

        /// <inheritdoc />
        public override void Configure(IConfigSection configSection)
        {
            _server = configSection.GetString(@"data/srtm/server");
            _schemaPath = configSection.GetString(@"data/srtm/schema");
            _elePath = configSection.GetString(@"data/elevation/local");
        }

        #endregion

        /// <inheritdoc />
        protected override string GetUri(QuadKey quadKey)
        {
            // TODO tile can cross more than one srtm cell: need load more.
            var prefix = GetFileNamePrefix(GeoUtils.QuadKeyToBoundingBox(quadKey).Center());
            foreach (var line in _fileSystemService.ReadText(_schemaPath).Split('\n'))
            {
                if (line.StartsWith(prefix))
                {
                    var parameters = line.Split(' ');
                    // NOTE some of files miss extension point between name and .hgt.zip
                    return String.Format("{0}/{1}/{2}", _server, ContinentMap[int.Parse(parameters[1])],
                        parameters[1].EndsWith("zip") ? "" : parameters[0] + ".hgt.zip");
                }
            }

            _trace.Warn(TraceCategory, String.Format("Cannot find {0} on {1}", prefix, _schemaPath));

            return null;
        }

        /// <inheritdoc />
        protected override string GetFilePath(QuadKey quadKey)
        {
            var prefix = GetFileNamePrefix(GeoUtils.QuadKeyToBoundingBox(quadKey).Center());
            return Path.Combine(_elePath, prefix);
        }

        /// <inheritdoc />
        protected override void WriteBytes(Stream stream, byte[] bytes)
        {
            _trace.Info(TraceCategory, "Unzipping {0} bytes", bytes.Length.ToString());
            var hgtData = CompressionUtils.Unzip(bytes).Single().Value;
            stream.Write(hgtData, 0, hgtData.Length);
        }

        private string GetFileNamePrefix(GeoCoordinate coordinate)
        {
            return String.Format("{0}{1:00}{2}{3:000}",
                coordinate.Latitude > 0 ? 'N' : 'S', Math.Abs((int)coordinate.Latitude),
                coordinate.Longitude > 0 ? 'E' : 'W', Math.Abs((int)coordinate.Longitude));
        }
    }
}
