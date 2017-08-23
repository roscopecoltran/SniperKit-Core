using UnityEngine;

namespace UtyMap.Unity.Animations.Path
{
    /// <summary>
    ///    A path interpolator defines API to get position/direction changes of an object.
    /// </summary>
    public interface IPathInterpolator
    {
        /// <summary> Gets object's point. </summary>
        Vector3 GetPoint(float value);

        /// <summary> Gets object's orientation. </summary>
        Vector3 GetDirection(float value);
    }
}
