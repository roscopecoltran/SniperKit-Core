using System.Collections.Generic;
using UtyRx;

namespace UtyMap.Unity.Infrastructure.IO
{
    public interface INetworkService
    {
        IObservable<string> Get(string url, Dictionary<string, string> headers = null);
        IObservable<byte[]> GetAndGetBytes(string url, Dictionary<string, string> headers = null);
    }
}
