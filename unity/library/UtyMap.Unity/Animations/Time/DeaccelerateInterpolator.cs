using System;

namespace UtyMap.Unity.Animations.Time
{
    /// <summary> 
    ///    An interpolator where the rate of change starts out quickly and and then decelerates.
    ///  </summary>
    public class DecelerateInterpolator : ITimeInterpolator
    {
        private readonly float _factor;

        public DecelerateInterpolator(float factor = 1)
        {
            _factor = factor;
        }

        /// <inheritdoc />
        public float GetTime(float value)
        {
            return Math.Abs(_factor - 1.0f) < float.Epsilon
                ? 1.0f - (1.0f - value) * (1.0f - value)
                : (float) (1.0f - Math.Pow((1.0f - value), 2 * _factor));
        }
    }
}
