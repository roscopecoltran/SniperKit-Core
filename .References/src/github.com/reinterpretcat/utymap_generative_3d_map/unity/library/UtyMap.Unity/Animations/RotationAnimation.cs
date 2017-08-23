using System;
using UnityEngine;
using UtyMap.Unity.Animations.Rotation;
using UtyMap.Unity.Animations.Time;

namespace UtyMap.Unity.Animations
{
    /// <summary> Animates transform rotation. </summary>
    public class RotationAnimation : TransformAnimation
    {
        private readonly IRotationInterpolator _rotationInterpolator;

        public RotationAnimation(Transform transform,
                                 ITimeInterpolator timeInterpolator,
                                 IRotationInterpolator rotationInterpolator,
                                 TimeSpan duration, bool isLoop = false)
            : base(transform, timeInterpolator, duration, isLoop)
        {
            _rotationInterpolator = rotationInterpolator;
        }

        /// <inheritdoc />
        protected override void UpdateTransform(Transform transform, float time)
        {
            transform.localRotation = _rotationInterpolator.GetRotation(time);
        }
    }
}
