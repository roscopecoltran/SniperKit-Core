using System;

namespace UtyMap.Unity.Animations
{
    /// <summary> Represents an animation. </summary>
    public abstract class Animation
    {
        /// <summary> Called when animation is finished. </summary>
        public event EventHandler Finished;

        /// <summary> True if animation is running. </summary>
        public bool IsRunning { get; private set; }

        /// <summary> Called when animation is started. </summary>
        protected internal abstract void OnStarted();

        /// <summary> Called when animation is updated. </summary>
        /// <param name="deltaTime"> Delta time between frames in seconds. </param>
        /// <remarks> Should be called externally. </remarks>
        protected internal abstract void OnUpdate(float deltaTime);

        /// <summary> Called when animation is stopped. </summary>
        /// <remarks> This might happen due manual stop or animation completion. </remarks>
        protected internal virtual void OnStopped(EventArgs e)
        {
            var @event = Finished;
            if (@event != null)
                @event(this, e);
        }
  
        /// <summary> Starts animation. </summary>
        public void Start()
        {
            if (!IsRunning)
            {
                IsRunning = true;
                OnStarted();
            }
        }

        /// <summary> Stops animation. </summary>
        public void Stop()
        {
            if (IsRunning)
            {
                IsRunning = false;
                OnStopped(EventArgs.Empty);
            }
        }
    }
}
