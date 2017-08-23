using System;
using System.Collections.Generic;
using System.Net;
using UtyRx;

namespace UtyMap.Unity.Infrastructure.IO
{
    /// <summary> Provides default implementation of <see cref="INetworkService" />. </summary>
    internal class NetworkService : INetworkService
    {
        /// <inheritdoc />
        public IObservable<string> Get(string url, Dictionary<string, string> headers = null)
        {
            var webClient = new WebClient();
            var query = Observable
                .FromEventPattern<DownloadStringCompletedEventHandler, DownloadStringCompletedEventArgs>
                (
                    eh => new DownloadStringCompletedEventHandler(eh),
                    eh => webClient.DownloadStringCompleted += eh,
                    eh => webClient.DownloadStringCompleted -= eh
                ).Select(o => o.EventArgs.Result);
            
            AddHeaders(webClient, headers);

            webClient.DownloadStringAsync(new Uri(url));
            return query;
        }

        /// <inheritdoc />
        public IObservable<byte[]> GetAndGetBytes(string url, Dictionary<string, string> headers = null)
        {
            var webClient = new WebClient();
            var query = Observable.FromEventPattern<DownloadDataCompletedEventHandler, DownloadDataCompletedEventArgs>
                (
                    eh => new DownloadDataCompletedEventHandler(eh),
                    eh => webClient.DownloadDataCompleted += eh,
                    eh => webClient.DownloadDataCompleted -= eh
                ).Select(o => o.EventArgs.Result);
            
            AddHeaders(webClient, headers);

            webClient.DownloadDataAsync(new Uri(url));
            return query;
        }

        private static void AddHeaders(WebClient webClient, Dictionary<string, string> headers)
        {
            if (headers == null)
                return;
            
            foreach (var header in headers)
                webClient.Headers.Add(header.Key, header.Value);
        }
    }
}