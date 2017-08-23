using System;
using System.Collections.Generic;
using NUnit.Framework;
using UtyMap.Unity.Animations;
using UtyMap.Unity.Animations.Time;

namespace UtyMap.Unity.Tests.Animations
{
    [TestFixture]
    public class CompositeAnimationTests
    {
        private FakeTransformAnimation _animationOne;
        private FakeTransformAnimation _animationTwo;
        private CompositeAnimation _compositeAnimation;

        [Test]
        public void CanStartAnimation()
        {
            CreateCompositeAnimation(TimeSpan.FromSeconds(1));

            _compositeAnimation.Start();

            Assert.IsTrue(_compositeAnimation.IsRunning);
        }

        [Test]
        public void CanStopRunningAnimation()
        {
            CreateCompositeAnimation(TimeSpan.FromSeconds(1));
            _compositeAnimation.Start();

            _compositeAnimation.Stop();

            Assert.IsFalse(_compositeAnimation.IsRunning);
        }

        [Test]
        public void CanUseTransformAnimationWithZeroDuration()
        {
            CreateCompositeAnimation(TimeSpan.Zero);
            _compositeAnimation.Start();

            _compositeAnimation.OnUpdate(1);

            Assert.IsFalse(_compositeAnimation.IsRunning);
            Assert.AreEqual(1, _animationOne.LastTime);
            Assert.AreEqual(1, _animationTwo.LastTime);
        }

        [Test]
        public void CanUseTransformAnimationWithNonZeroDuration()
        {
            const float value = 0.5f;
            CreateCompositeAnimation(TimeSpan.FromSeconds(1));
            _compositeAnimation.Start();

            _compositeAnimation.OnUpdate(value);

            Assert.IsTrue(_compositeAnimation.IsRunning);
            Assert.AreEqual(value, _animationOne.LastTime);
            Assert.AreEqual(value, _animationTwo.LastTime);
        }

        private void CreateCompositeAnimation(TimeSpan timeSpan)
        {
            _animationOne = new FakeTransformAnimation(new LinearInterpolator(), timeSpan);
            _animationTwo = new FakeTransformAnimation(new LinearInterpolator(), timeSpan);
            _compositeAnimation = new CompositeAnimation(new List<Animation> { _animationOne, _animationTwo });
        }
    }
}
