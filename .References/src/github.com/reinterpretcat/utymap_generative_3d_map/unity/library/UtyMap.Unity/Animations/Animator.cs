using System;
using UtyMap.Unity.Animations.Time;

namespace UtyMap.Unity.Animations
{
    /// <summary> Provides the way to handle animation. </summary>
    public abstract class Animator
    {
        private Animation _animation;

        /// <summary> Called when animation is updated. </summary>
        protected abstract void OnAnimationUpdate(float deltaTime);

        /// <summary> Animates to given coordinate using given timeinterpolator and duration. </summary>
        public abstract void AnimateTo(GeoCoordinate coordinate, float zoom, TimeSpan duration, ITimeInterpolator timeInterpolator);

        /// <summary> Notifies animator about frame update. </summary>
        public void Update(float deltaTime)
        {
            if (_animation == null || !_animation.IsRunning)
                return;

            OnAnimationUpdate(deltaTime);

            _animation.OnUpdate(deltaTime);
        }

        /// <summary> True if there is running animation. </summary>
        public bool IsRunningAnimation
        {
            get { return _animation != null && _animation.IsRunning; }
        }

        /// <summary> Starts animation if it is set. </summary>
        public void Start()
        {
            if (_animation != null)
                _animation.Start();
        }

        /// <summary> Cancels outstanding animation. </summary>
        public void Cancel()
        {
            if (_animation != null)
                _animation.Stop();
        }

        /// <summary> Sets animation. </summary>
        /// <remarks> Stops existing animation. </remarks>
        protected void SetAnimation(Animation animation)
        {
            Cancel();
            _animation = animation;
        }
    }
}
