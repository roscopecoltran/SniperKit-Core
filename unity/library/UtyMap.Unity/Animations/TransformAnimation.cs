using System;
using UnityEngine;
using UtyMap.Unity.Animations.Time;

namespace UtyMap.Unity.Animations
{
    /// <summary> Represents animation for updating transform. </summary>
    public abstract class TransformAnimation : Animation
    {
        private readonly Transform _transform;
        private readonly ITimeInterpolator _timeInterpolator;
        private readonly float _duration;
        private readonly bool _isLoop;
        private float _time;

        protected TransformAnimation(Transform transform,
                                     ITimeInterpolator timeInterpolator,
                                     TimeSpan duration, bool isLoop = false)
        {
            _transform = transform;
            _timeInterpolator = timeInterpolator;
            _duration = (float) duration.TotalSeconds;
            _isLoop = isLoop;
        }

        /// <summary> Updates transform using interpolated time. </summary>
        protected abstract void UpdateTransform(Transform transform, float time);

        /// <inheritdoc />
        protected internal override void OnStarted()
        {
            _time = 0;
        }

        /// <inheritdoc />
        protected internal override void OnUpdate(float deltaTime)
        {
            if (Math.Abs(_duration) < float.Epsilon)
            {
                UpdateTransform(_transform, _timeInterpolator.GetTime(1));
                Stop();
                return;
            }
               
            _time += deltaTime / _duration;

            if (_time > 1)
            {
                if (_isLoop)
                    _time = 0;
                else
                {
                    Stop();
                    return;
                }
            }

            UpdateTransform(_transform, _timeInterpolator.GetTime(_time));
        }
    }
}
