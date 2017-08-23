using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using Moq;
using NUnit.Framework;
using UtyDepend.Config;
using UtyMap.Unity.Data;
using UtyMap.Unity.Data.Providers.Elevation;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Tests.Helpers;
using UtyMap.Unity.Utils;
using UtyRx;

namespace UtyMap.Unity.Tests.Providers
{
    [TestFixture]
    public class MapzenElevationDataProviderTests
    {
        private Mock<IFileSystemService> _fileSystemService;
        private Mock<INetworkService> _networkService;
        private Mock<ITrace> _trace;
        private Mock<IConfigSection> _config;
        private Mock<Stream> _writeStream;
        
        private Tile _tile;
        private byte[] _responseBytes;

        private MapzenElevationDataProvider _eleProvider;

        [TestFixtureSetUp]
        public void SetUp()
        {
            _fileSystemService = new Mock<IFileSystemService>();
            _networkService = new Mock<INetworkService>();
            _trace = new Mock<ITrace>();

            _config = new Mock<IConfigSection>();
            _config.Setup(c => c.GetString("data/mapzen/ele_server", It.IsAny<string>())).Returns("elevation.mapzen.com/height?json={0}&api_key={1}");
            _config.Setup(c => c.GetString("data/mapzen/api_key", It.IsAny<string>())).Returns("ggg");
            _config.Setup(c => c.GetString("data/mapzen/ele_format", It.IsAny<string>())).Returns("ele");
            _config.Setup(c => c.GetBool("data/mapzen/limit", It.IsAny<bool>())).Returns(false);
            _config.Setup(c => c.GetInt("data/mapzen/ele_grid", It.IsAny<int>())).Returns(2);
            _config.Setup(c => c.GetString("data/elevation/local", It.IsAny<string>())).Returns("index/data");

            _writeStream = new Mock<Stream>();
            _responseBytes = Encoding.UTF8.GetBytes("{\"encoded_polyline\":\"kzcecBqdapX?sjD?ujDmgBhvI?sjD?ujDmgBhvI?sjD?ujD\",\"height\":[43,38,37,37]}");

            _tile = new Tile(GeoUtils.CreateQuadKey(TestHelper.WorldZeroPoint, 16),
                new Stylesheet(""), new CartesianProjection(TestHelper.WorldZeroPoint), ElevationDataType.Flat);

            _networkService
                .Setup(ns => ns.GetAndGetBytes(It.IsAny<string>(), It.IsAny<Dictionary<string,string>>()))
                .Returns(Observable.Return(_responseBytes));

            _fileSystemService
                .Setup(fs => fs.WriteStream(It.IsAny<string>()))
                .Returns(_writeStream.Object);
            
            _eleProvider = new MapzenElevationDataProvider(_fileSystemService.Object, _networkService.Object, _trace.Object);

            _eleProvider.Configure(_config.Object);
        }

        [Test]
        public void CanRequestPolyline()
        {
            _eleProvider.GetResultSync(_tile);

            _networkService.Verify(ns => ns.GetAndGetBytes("elevation.mapzen.com/height?json={\"range\":false,\"encoded_polyline\":\"kzcecBqdapX?sjD?ujDmgBhvI?sjD?ujDmgBhvI?sjD?ujD\"}&api_key=ggg", It.IsAny<Dictionary<string, string>>()));
        }

        [Test]
        public void CanCreateDataFile()
        {
            _eleProvider.GetResultSync(_tile);

            _fileSystemService.Verify(fs => fs.WriteStream(Path.Combine("index/data", Path.Combine("16", "1202102332220103.ele"))));
        }

        [Test]
        public void CanGetElevationFilePath()
        {
            var result = _eleProvider.GetResultSync(_tile);

            Assert.AreEqual(Path.Combine("index/data", Path.Combine("16", "1202102332220103.ele")), result.Item2);
        }

        [Test]
        public void CanWriteCorrectEleData()
        {
            var expectedBytes = Encoding.UTF8.GetBytes("43 38 37 37");

            _eleProvider.GetResultSync(_tile);

            _writeStream.Verify(ws => ws.Write(expectedBytes, 0, expectedBytes.Length));
        }
    }
}
