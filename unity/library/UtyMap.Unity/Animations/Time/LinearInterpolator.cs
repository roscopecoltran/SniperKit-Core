namespace UtyMap.Unity.Animations.Time
{
    /// <summary> An interpolator where the rate of change is constant . </summary>
    public class LinearInterpolator : ITimeInterpolator
    {
        /// <inheritdoc />
        public float GetTime(float value)
        {
            return value;
        }
    }
}
