using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

namespace UtyMap.Unity.Animations.Rotation
{
    /// <summary> Interpolates object's rotation using linear interpolation. </summary>
    public class LinearInterpolator : IRotationInterpolator
    {
        private readonly Quaternion[] _quaternions;

        public LinearInterpolator(IEnumerable<Quaternion> quaternions)
        {
            _quaternions = quaternions.ToArray();

            if (_quaternions.Length > 2)
                throw new NotImplementedException();

            if (_quaternions.Length < 2)
                throw new ArgumentException("LinearInterpolator does not support less than 2 quaternions.");
        }

        /// <inheritdoc />
        public Quaternion GetRotation(float time)
        {
            return Quaternion.Slerp(_quaternions[0], _quaternions[1], time);
        }
    }
}
