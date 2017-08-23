using System;

namespace UtyMap.Unity.Animations.Time
{
    /// <summary>
    ///  A time interpolator where the rate of change starts and ends slowly but
    ///  accelerates through the middle.
    /// </summary>
    public class AccelerateDecelerateInterpolator : ITimeInterpolator
    {
        /// <inheritdoc />
        public float GetTime(float value)
        {
            return (float) (Math.Cos((value + 1) * Math.PI) / 2.0f) + 0.5f;
        }
    }
}
