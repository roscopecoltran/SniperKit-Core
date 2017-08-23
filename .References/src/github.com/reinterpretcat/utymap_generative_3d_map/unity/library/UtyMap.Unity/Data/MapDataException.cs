using System;
using System.Diagnostics.CodeAnalysis;

namespace UtyMap.Unity.Data
{
    [SuppressMessage("Microsoft.Usage", "CA2240:ImplementISerializableCorrectly")]
    [Serializable]
    internal class MapDataException : Exception
    {
        /// <summary> Creates <see cref="MapDataException" />. </summary>
        /// <param name="message">Exception message.</param>
        public MapDataException(string message)
            : base(message)
        {
        }

        /// <summary> Creates <see cref="MapDataException" />. </summary>
        /// <param name="formatStr">Formatting string.</param>
        /// <param name="message">Exception message.</param>
        public MapDataException(string formatStr, string message)
            : base(String.Format(formatStr, message))
        {
        }
    }
}