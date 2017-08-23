using System;

namespace UtyMap.Unity.Animations.Time
{
    /// <summary>
    ///    An interpolator where the rate of change starts out slowly and and then accelerates.
    /// </summary>
    public class AccelerateInterpolator : ITimeInterpolator
    {
        private readonly float _factor;
        private readonly float _doubleFactor;

        public AccelerateInterpolator(float factor = 1)
        {
            _factor = factor;
            _doubleFactor = 2 * factor;
        }

        /// <inheritdoc />
        public float GetTime(float value)
        {
            return Math.Abs(_factor - 1.0f) < float.Epsilon
                ? value * value
                : (float) Math.Pow(value, _doubleFactor);
        }
    }
}
