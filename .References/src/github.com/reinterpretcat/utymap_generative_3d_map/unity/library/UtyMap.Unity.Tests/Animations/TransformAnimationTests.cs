using System;
using NUnit.Framework;
using UtyMap.Unity.Animations.Time;

namespace UtyMap.Unity.Tests.Animations
{
    [TestFixture]
    public class TransformAnimationTests
    {
        [Test]
        public void AnimationIsNotStartedAutomatically()
        {
            var transformAnimation = CreateAnimation(TimeSpan.FromSeconds(1), false);

            Assert.IsFalse(transformAnimation.IsRunning);
        }

        [Test]
        public void CanStartAnimation()
        {
            var transformAnimation = CreateAnimation(TimeSpan.FromSeconds(1), false);

            transformAnimation.Start();

            Assert.IsTrue(transformAnimation.IsRunning);
        }

        [Test]
        public void CanStopAnimation()
        {
            var transformAnimation = CreateAnimation(TimeSpan.FromSeconds(1), false);
            transformAnimation.Start();

            transformAnimation.Stop();

            Assert.IsFalse(transformAnimation.IsRunning);
        }

        [Test]
        public void WhenValueGreaterThanOne_OnUpdate_StopsUnloopedAnimation()
        {
            var transformAnimation = CreateAnimation(TimeSpan.FromSeconds(1), false);
            transformAnimation.Start();

            transformAnimation.OnUpdate(1.1f);

            Assert.IsFalse(transformAnimation.IsRunning);
        }

        [Test]
        public void WhenValueGreaterThanOne_OnUpdate_ContinuesLoopedAnimation()
        {
            var transformAnimation = CreateAnimation(TimeSpan.FromSeconds(1), true);
            transformAnimation.Start();

            transformAnimation.OnUpdate(1.1f);

            Assert.IsTrue(transformAnimation.IsRunning);
            Assert.AreEqual(0f, transformAnimation.LastTime);
        }

        private FakeTransformAnimation CreateAnimation(TimeSpan timeSpan, bool isLoop)
        {
            return new FakeTransformAnimation(new LinearInterpolator(), timeSpan, isLoop);
        }
    }
}
