using System.IO;
using Assets.Scripts.Environment;
using UtyMap.Unity.Infrastructure.IO;

namespace Assets.Scripts.Core.Plugins
{
    internal class UnityPathResolver : IPathResolver
    {
        private readonly string _root = EnvironmentApi.ExternalDataPath;

        /// <inheritdoc />
        public string Resolve(string path)
        {
            return path.StartsWith(_root) ? path : Path.Combine(_root, path);
        }
    }
}
