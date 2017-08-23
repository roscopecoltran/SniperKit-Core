using System;
using System.Collections.Generic;
using Assets.Scripts.Scenes.Map.Tiling;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Animations;
using UtyMap.Unity.Animations.Time;
using Animator = UtyMap.Unity.Animations.Animator;
using Animation = UtyMap.Unity.Animations.Animation;

namespace Assets.Scripts.Scenes.Map.Animations
{
    internal abstract class SpaceAnimator : Animator
    {
        protected readonly Transform Pivot;
        protected readonly Transform Camera;
        protected readonly TileController TileController;

        /// <summary> Keeps track of the last animation. </summary>
        private AnimationState _lastState;

        /// <summary> Creates animation for given coordinate and zoom level with given duration. </summary>
        protected abstract Animation CreateAnimationTo(GeoCoordinate coordinate, float zoom, TimeSpan duration, ITimeInterpolator timeInterpolator);

        protected SpaceAnimator(TileController tileController)
        {
            Pivot = tileController.Pivot;
            Camera = Pivot.Find("Camera").transform;
            TileController = tileController;
        }

        /// <inheritdoc />
        public sealed override void AnimateTo(GeoCoordinate coordinate, float zoom, TimeSpan duration, ITimeInterpolator timeInterpolator)
        {
            _lastState = new AnimationState(coordinate, zoom, duration, timeInterpolator);

            SetAnimation(CreateAnimationTo(coordinate, zoom, duration, timeInterpolator));
            Start();
        }

        /// <summary> Continues animation. </summary>
        public void ContinueFrom(SpaceAnimator other)
        {
            var state = other._lastState;
            AnimateTo(state.Coordinate, state.Zoom, TimeSpan.FromSeconds(state.TimeLeft), state.TimeInterpolator);
        }

        /// <inheritdoc />
        protected sealed override void OnAnimationUpdate(float deltaTime)
        {
            _lastState.OnUpdate(deltaTime);
        }

        protected PathAnimation CreatePathAnimation(Transform target, TimeSpan duration, ITimeInterpolator timeInterpolator, IEnumerable<Vector3> points)
        {
            return new PathAnimation(target, timeInterpolator,
                new UtyMap.Unity.Animations.Path.LinearInterpolator(points),
                duration);
        }

        protected RotationAnimation CreateRotationAnimation(Transform target, TimeSpan duration, ITimeInterpolator timeInterpolator, IEnumerable<Quaternion> rotations)
        {
            return new RotationAnimation(target, timeInterpolator,
                new UtyMap.Unity.Animations.Rotation.LinearInterpolator(rotations),
                duration);
        }

        #region Nested classes

        /// <summary> Keeps state of last animation. </summary>
        /// <remarks> Introduced to support animation transition from one space to another. </remarks>
        struct AnimationState
        {
            public readonly GeoCoordinate Coordinate;
            public readonly float Zoom;
            public ITimeInterpolator TimeInterpolator;
            public float TimeLeft;

            public AnimationState(GeoCoordinate coordinate, float zoom, TimeSpan duration, ITimeInterpolator timeInterpolator)
            {
                Coordinate = coordinate;
                Zoom = zoom;
                TimeInterpolator = timeInterpolator;
                TimeLeft = (float) duration.TotalSeconds;
            }

            public void OnUpdate(float deltaTime) { TimeLeft -= deltaTime; }
        }

        #endregion
    }
}
