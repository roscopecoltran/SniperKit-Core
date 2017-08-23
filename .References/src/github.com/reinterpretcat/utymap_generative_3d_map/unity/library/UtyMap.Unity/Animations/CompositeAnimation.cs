using System;
using System.Collections.Generic;
using System.Linq;

namespace UtyMap.Unity.Animations
{
    /// <summary> Provides the way to compose more than one animation. </summary>
    public class CompositeAnimation : Animation
    {
        private readonly IEnumerable<Animation> _animations;

        public CompositeAnimation(IEnumerable<Animation> animations)
        {
            _animations = animations;
        }

        /// <inheritdoc />
        protected internal override void OnStarted()
        {
            foreach (var animation in _animations)
                animation.Start();
        }

        /// <inheritdoc />
        protected internal override void OnStopped(EventArgs e)
        {
            foreach (var animation in _animations)
                animation.Stop();
        }

        /// <inheritdoc />
        protected internal override void OnUpdate(float deltaTime)
        {
            bool hasRunning = false;
            foreach (var animation in _animations)
            {
                if (!animation.IsRunning)
                    continue;

                animation.OnUpdate(deltaTime);
                hasRunning |= animation.IsRunning;
            }

            if (!hasRunning)
                Stop();
        }
    }
}
