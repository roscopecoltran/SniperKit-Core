using System.IO;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyDepend;

namespace UtyMap.Unity.Infrastructure.IO
{
    /// <summary> Provides a way to interact with file system. </summary>
    public class FileSystemService : IFileSystemService
    {
        protected const string LogTag = "file";

        protected readonly IPathResolver PathResolver;
        protected readonly ITrace Trace;

        /// <summary> Creates <see cref="FileSystemService"/>. </summary>
        /// <param name="pathResolver">Path resolver.</param>
        /// <param name="trace">Trace.</param>
        [Dependency]
        public FileSystemService(IPathResolver pathResolver, ITrace trace)
        {
            PathResolver = pathResolver;
            Trace = trace;
        }

        /// <inheritdoc />
        public Stream ReadStream(string path)
        {
            var resolvedPath = PathResolver.Resolve(path);
            Trace.Debug(LogTag, "read stream from {0}", resolvedPath);
            return File.Open(resolvedPath, FileMode.Open,
                FileAccess.Read, FileShare.Read);
        }

        public Stream WriteStream(string path)
        {
            var resolvedPath = PathResolver.Resolve(path);
            Trace.Debug(LogTag, "write stream from {0}", resolvedPath);
            return new FileStream(resolvedPath, FileMode.Create);
        }

        /// <inheritdoc />
        public string ReadText(string path)
        {
            var resolvedPath = PathResolver.Resolve(path);
            Trace.Debug(LogTag, "read text from {0}", resolvedPath);
            using (var reader = new StreamReader(resolvedPath))
                return reader.ReadToEnd();
        }

        /// <inheritdoc />
        public byte[] ReadBytes(string path)
        {
            var resolvedPath = PathResolver.Resolve(path);
            Trace.Debug(LogTag, "read bytes from {0}", resolvedPath);
            return File.ReadAllBytes(resolvedPath);
        }

        /// <inheritdoc />
        public bool Exists(string path)
        {
            var resolvedPath = PathResolver.Resolve(path);
            Trace.Debug(LogTag, "checking {0}", resolvedPath);
            return File.Exists(resolvedPath);
        }

        /// <inheritdoc />
        public virtual string[] GetFiles(string path, string searchPattern)
        {
            var resolvedPath = PathResolver.Resolve(path);
            Trace.Debug(LogTag, "getting files from {0}", resolvedPath);
            return Directory.GetFiles(resolvedPath, searchPattern);
        }

        /// <inheritdoc />
        public virtual string[] GetDirectories(string path, string searchPattern)
        {
            var resolvedPath = PathResolver.Resolve(path);
            Trace.Debug(LogTag, "getting directories from {0}", resolvedPath);
            return Directory.GetDirectories(resolvedPath, searchPattern);
        }

        public virtual void CreateDirectory(string path)
        {
            var resolvedPath = PathResolver.Resolve(path);
            Trace.Debug(LogTag, "create directory: {0}", resolvedPath);
            Directory.CreateDirectory(resolvedPath);
        }
    }
}
