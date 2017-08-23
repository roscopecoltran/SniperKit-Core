using System.Collections.Generic;
using Assets.Scripts.Environment.Reactive;
using UtyMap.Unity.Infrastructure.IO;
using UtyRx;

namespace Assets.Scripts.Core.Plugins
{
    internal class UnityNetworkService : INetworkService
    {
        /// <inheritdoc />
        public IObservable<string> Get(string url, Dictionary<string, string> headers = null)
        {
            return ObservableWWW.Get(url, headers);
        }

        /// <inheritdoc />
        public IObservable<byte[]> GetAndGetBytes(string url, Dictionary<string, string> headers = null)
        {
            return ObservableWWW.GetAndGetBytes(url, headers);
        }
    }
}