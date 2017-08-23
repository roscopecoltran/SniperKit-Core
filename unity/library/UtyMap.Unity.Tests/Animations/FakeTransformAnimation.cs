using System;
using UnityEngine;
using UtyMap.Unity.Animations;
using UtyMap.Unity.Animations.Time;

namespace UtyMap.Unity.Tests.Animations
{
    class FakeTransformAnimation : TransformAnimation
    {
        public float LastTime { get; private set; }

        public FakeTransformAnimation(ITimeInterpolator timeInterpolator, TimeSpan duration, bool isLoop = false) :
            base(null, timeInterpolator, duration, isLoop)
        {
        }

        protected override void UpdateTransform(Transform transform, float time)
        {
            LastTime = time;
        }
    }
}
