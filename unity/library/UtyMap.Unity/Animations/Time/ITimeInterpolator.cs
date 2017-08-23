namespace UtyMap.Unity.Animations.Time
{
    /// <summary>
    ///     A time interpolator defines the rate of time change of an animation. This allows animations
    ///     to have non-linear motion, such as acceleration and deceleration.
    /// </summary>
    public interface ITimeInterpolator
    {
        /// <summary>
        ///     Maps a value representing the elapsed fraction of an animation to a value that 
        ///     represents the interpolated fraction.
        /// </summary>
        float GetTime(float value);
    }
}
