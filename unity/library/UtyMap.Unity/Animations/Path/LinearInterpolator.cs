using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

namespace UtyMap.Unity.Animations.Path
{
    /// <summary> Interpolates object's path using linear interpolation. </summary>
    public class LinearInterpolator : IPathInterpolator
    {
        private readonly Vector3[] _points;

        public LinearInterpolator(IEnumerable<Vector3> points)
        {
            _points = points.ToArray();

            if (_points.Length > 2)
                throw new NotImplementedException();

            if (_points.Length < 2)
                throw new ArgumentException("LinearInterpolator does not support less than 2 points.");
        }

        /// <inheritdoc />
        public Vector3 GetPoint(float value)
        {
            return Vector3.Lerp(_points[0], _points[1], value);
        }

        /// <inheritdoc />
        public Vector3 GetDirection(float value)
        {
            return (_points[1] - _points[0]).normalized;
        }
    }
}
