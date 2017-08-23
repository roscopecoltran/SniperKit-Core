using System.IO;
using UtyMap.Unity.Infrastructure.IO;

namespace UtyMap.Unity.Tests.Helpers
{
    public class TestPathResolver : IPathResolver
    {
        public string Resolve(string path)
        {
            return path.StartsWith("..") 
                ? path 
                : Path.Combine(TestHelper.TestAssetsFolder, path);
        }
    }
}
