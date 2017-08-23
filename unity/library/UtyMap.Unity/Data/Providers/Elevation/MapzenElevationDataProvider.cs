using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.Formats;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Utils;

namespace UtyMap.Unity.Data.Providers.Elevation
{
    internal class MapzenElevationDataProvider : RemoteMapDataProvider
    {
        private string _mapDataServerUri;
        private string _mapDataApiKey;
        private string _elePath;
        private int _eleGrid;
        private string _mapDataFormatExtension;

        private bool _hasRequestLimit = true;

        [Dependency]
        public MapzenElevationDataProvider(IFileSystemService fileSystemService, INetworkService networkService, ITrace trace) :
            base(fileSystemService, networkService, trace)
        {
        }

        /// <inheritdoc />
        protected override string GetUri(QuadKey quadKey)
        {
            return String.Format(_mapDataServerUri, GetJsonPayload(quadKey), _mapDataApiKey);
        }

        /// <inheritdoc />
        protected override string GetFilePath(QuadKey quadKey)
        {
            var directory = Path.Combine(_elePath, quadKey.LevelOfDetail.ToString());
            return Path.Combine(directory, quadKey + _mapDataFormatExtension);
        }

        /// <inheritdoc />
        protected override bool LimitRequests { get { return _hasRequestLimit; } }

        /// <inheritdoc />
        protected override void WriteBytes(Stream stream, byte[] bytes)
        {
            var content = Encoding.UTF8.GetString(bytes);
            var json = JSON.Parse(content);
            var heights = json["height"].AsArray;

            StringBuilder sb = new StringBuilder(1024);
            foreach (JSONNode jsonHeight in heights)
                sb.AppendFormat("{0} ", jsonHeight.Value);
            sb.Remove(sb.Length - 1, 1);

            var intBytes = Encoding.UTF8.GetBytes(sb.ToString());
            
            stream.Write(intBytes, 0, intBytes.Length);
        }

        private string GetJsonPayload(QuadKey quadkey)
        {
            return String.Format("{{\"range\":false,\"encoded_polyline\":\"{0}\"}}",
                EncodePolyline(CreatePolyline(quadkey)));
        }

        private IEnumerable<GeoCoordinate> CreatePolyline(QuadKey quadkey)
        {
            var bbox = GeoUtils.QuadKeyToBoundingBox(quadkey);

            var latStep = (bbox.MaxPoint.Latitude - bbox.MinPoint.Latitude) / _eleGrid;
            var lonStep = (bbox.MaxPoint.Longitude - bbox.MinPoint.Longitude) / _eleGrid;

            for (int i = 0; i <= _eleGrid; ++i)
            {
                var lat = bbox.MinPoint.Latitude + i * latStep;
                for (int j = 0; j <= _eleGrid; ++j)
                {
                    var lon = bbox.MinPoint.Longitude + j * lonStep;
                    yield return new GeoCoordinate(lat, lon);
                }
            }
        }

        private static string EncodePolyline(IEnumerable<GeoCoordinate> points)
        {
            const double precision = 1E6;

            var str = new StringBuilder();
            Action<int> encodeDiff = (diff =>
            {
                int shifted = diff << 1;
                if (diff < 0)
                    shifted = ~shifted;
                int rem = shifted;
                while (rem >= 0x20)
                {
                    str.Append((char) ((0x20 | (rem & 0x1f)) + 63));
                    rem >>= 5;
                }
                str.Append((char) (rem + 63));
            });

            int lastLat = 0;
            int lastLng = 0;
            foreach (var point in points)
            {
                int lat = (int) Math.Round(point.Latitude*precision);
                int lng = (int) Math.Round(point.Longitude*precision);
                encodeDiff(lat - lastLat);
                encodeDiff(lng - lastLng);
                lastLat = lat;
                lastLng = lng;
            }
            return str.ToString();
        }

        /// <inheritdoc />
        public override void Configure(IConfigSection configSection)
        {
            _mapDataServerUri = configSection.GetString(@"data/mapzen/ele_server");
            _mapDataApiKey = configSection.GetString(@"data/mapzen/api_key");
            _mapDataFormatExtension = "." + configSection.GetString(@"data/mapzen/ele_format", "ele");

            // TODO this parameter depends on height data precision and tile size
            _eleGrid = configSection.GetInt(@"data/mapzen/ele_grid", 25);
            _elePath = configSection.GetString(@"data/elevation/local", null);

            _hasRequestLimit = configSection.GetBool(@"data/mapzen/limit", true);
        }
    }
}
