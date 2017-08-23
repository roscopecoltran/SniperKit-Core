using System;
using NUnit.Framework;
using UtyMap.Unity.Infrastructure.Diagnostic;

namespace UtyMap.Unity.Tests.Infrastructure.Diagnostic
{
    [TestFixture]
    public class DefaultTraceTests
    {
        private const string TestCategory = "category";
        private const string TestMessage = "message";

        private class TestDefaultTrace : DefaultTrace
        {
            public int Counter { get; private set; }
            public string Message { get; private set; }
            public string Category { get; private set; }

            public TestDefaultTrace(TraceLevel level) : base(level)
            {
            } 

            protected override void OnWriteRecord(TraceLevel type, string category, string message, Exception exception)
            {
                ++Counter;
                Category = category;
                Message = message;
            }
        }

        [Test]
        public void CanLogDebug()
        {
            CanLog(DefaultTrace.TraceLevel.Debug, (trace) => trace.Debug(TestCategory, TestMessage));
        }

        [Test]
        public void CanLogInfo()
        {
            CanLog(DefaultTrace.TraceLevel.Info, (trace) => trace.Info(TestCategory, TestMessage));
        }

        [Test]
        public void CanLogWarn()
        {
            CanLog(DefaultTrace.TraceLevel.Warn, (trace) => trace.Warn(TestCategory, TestMessage));
        }

        [Test]
        public void CanLogError()
        {
            CanLog(DefaultTrace.TraceLevel.Error, (trace) => trace.Error(TestCategory, null, TestMessage));
        }

        [Test]
        public void CanUseErrorTraceLevel()
        {
            var trace = new TestDefaultTrace(DefaultTrace.TraceLevel.Error);

            trace.Debug(TestCategory, trace.Category);
            trace.Info(TestCategory, trace.Category);
            trace.Warn(TestCategory, trace.Category);

            Assert.AreEqual(0, trace.Counter);

            trace.Error(TestCategory, null, trace.Category);
            Assert.AreEqual(1, trace.Counter);
        }

        [Test]
        public void CanUseWarnTraceLevel()
        {
            var trace = new TestDefaultTrace(DefaultTrace.TraceLevel.Warn);

            trace.Debug(TestCategory, trace.Category);
            trace.Info(TestCategory, trace.Category);

            Assert.AreEqual(0, trace.Counter);

            trace.Warn(TestCategory, trace.Category);
            trace.Error(TestCategory, null, trace.Category);
            Assert.AreEqual(2, trace.Counter);
        }

        [Test]
        public void CanUseInfoTraceLevel()
        {
            var trace = new TestDefaultTrace(DefaultTrace.TraceLevel.Info);

            trace.Debug(TestCategory, trace.Category);

            Assert.AreEqual(0, trace.Counter);

            trace.Info(TestCategory, trace.Category);
            trace.Warn(TestCategory, trace.Category);
            trace.Error(TestCategory, null, trace.Category);
            Assert.AreEqual(3, trace.Counter);
        }

        [Test]
        public void CanUseDebugTraceLevel()
        {
            var trace = new TestDefaultTrace(DefaultTrace.TraceLevel.Debug);

            trace.Debug(TestCategory, trace.Category);
            trace.Info(TestCategory, trace.Category);
            trace.Warn(TestCategory, trace.Category);
            trace.Error(TestCategory, null, trace.Category);

            Assert.AreEqual(4, trace.Counter);
        }

        private void CanLog(DefaultTrace.TraceLevel level, Action<TestDefaultTrace> action)
        {
            var trace = new TestDefaultTrace(level);

            action(trace);

            Assert.AreEqual(1, trace.Counter);
            Assert.AreEqual(TestCategory, trace.Category);
            Assert.AreEqual(TestMessage, trace.Message);
        }
    }
}
