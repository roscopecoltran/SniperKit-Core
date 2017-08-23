using UnityEngine;

namespace UtyMap.Unity.Animations.Rotation
{
    /// <summary>
    ///    A path interpolator defines API to get rotation changes of an object.
    /// </summary>
    public interface IRotationInterpolator
    {
        /// <summary> Gets object's rotation. </summary>
        Quaternion GetRotation(float time);
    }
}
