using System;
using UnityEngine;
using UtyMap.Unity.Animations.Path;
using UtyMap.Unity.Animations.Time;

namespace UtyMap.Unity.Animations
{
    /// <summary> Animates transform position. </summary>
    public class PathAnimation : TransformAnimation
    {
        private readonly IPathInterpolator _pathInterpolator;

        public PathAnimation(Transform transform, 
                             ITimeInterpolator timeInterpolator,
                             IPathInterpolator pathInterpolator,
                             TimeSpan duration, bool isLoop = false)
            : base(transform, timeInterpolator, duration, isLoop)
        {
            _pathInterpolator = pathInterpolator;
        }

        /// <inheritdoc />
        protected override void UpdateTransform(Transform transform, float time)
        {
            transform.localPosition = _pathInterpolator.GetPoint(time);
        }
    }
}
