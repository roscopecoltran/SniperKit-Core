using System;
using UtyMap.Unity.Infrastructure.Diagnostic;

namespace UtyMap.Unity.Tests.Helpers
{
    public class ConsoleTrace: DefaultTrace
    {
        public ConsoleTrace()
            : base(TraceLevel.Debug | TraceLevel.Info | TraceLevel.Warn | TraceLevel.Error)
        {
        }

        protected override void OnWriteRecord(TraceLevel type, string category, string message, Exception exception)
        {
            Console.WriteLine("[{0}] {1}: {2}{3}", type, category, message,
                (exception == null? "": " .Exception:" + exception));
        }
    }
}
