using System;
using UtyMap.Unity.Infrastructure.Diagnostic;

namespace Assets.Scripts.Core.Plugins
{
    internal sealed class UnityLogTrace : DefaultTrace
    {
        protected override void OnWriteRecord(TraceLevel type, string category, string message, Exception exception)
        {
            switch (type)
            {
                case TraceLevel.Error:
                    UnityEngine.Debug.LogError(String.Format("[{0}] {1}:{2}. Exception: {3}", type, category, message, exception));
                    break;
                case TraceLevel.Warn:
                    UnityEngine.Debug.LogWarning(String.Format("[{0}] {1}:{2}", type, category, message));
                    break;
                default:
                    UnityEngine.Debug.Log(String.Format("[{0}] {1}: {2}", type, category, message));
                    break;
            }
        }
    }
}
