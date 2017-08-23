using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

namespace UtyMap.Unity.Animations.Path
{
    /// <summary> Interpolates object's path using splines. </summary>
    public class SplineInterpolator : IPathInterpolator
    {
        private readonly BezierSpline _spline;

        public SplineInterpolator(IEnumerable<Vector3> points)
        {
            _spline = new BezierSpline(points.ToList());
        }

        /// <inheritdoc />
        public Vector3 GetPoint(float value)
        {
            return _spline.GetPoint(value);
        }

        /// <inheritdoc />
        public Vector3 GetDirection(float value)
        {
            return _spline.GetDirection(value);
        }

        #region Nested classes

        private static class Bezier
        {
            public static Vector3 GetPoint(Vector3 p0, Vector3 p1, Vector3 p2, float t)
            {
                t = Mathf.Clamp01(t);
                float oneMinusT = 1f - t;
                return oneMinusT * oneMinusT * p0 +
                       2f * oneMinusT * t * p1 +
                       t * t * p2;
            }

            public static Vector3 GetFirstDerivative(Vector3 p0, Vector3 p1, Vector3 p2, float t)
            {
                return 2f * (1f - t) * (p1 - p0) +
                       2f * t * (p2 - p1);
            }

            public static Vector3 GetPoint(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float t)
            {
                t = Mathf.Clamp01(t);
                float OneMinusT = 1f - t;
                return OneMinusT * OneMinusT * OneMinusT * p0 +
                       3f * OneMinusT * OneMinusT * t * p1 +
                       3f * OneMinusT * t * t * p2 +
                       t * t * t * p3;
            }

            public static Vector3 GetFirstDerivative(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float t)
            {
                t = Mathf.Clamp01(t);
                float oneMinusT = 1f - t;
                return 3f * oneMinusT * oneMinusT * (p1 - p0) +
                       6f * oneMinusT * t * (p2 - p1) +
                       3f * t * t * (p3 - p2);
            }
        }

        private class BezierSpline
        {
            private readonly Vector3[] _points;

            public BezierSpline(IEnumerable<Vector3> points)
            {
                _points = points.ToArray();

                if (_points.Length < 3)
                    throw new ArgumentException("Spline cannot be constructed from less than 3 points.");
            }

            public Vector3 GetPoint(float t)
            {
                if (_points.Length == 3)
                    return Bezier.GetPoint(_points[0], _points[1], _points[2], t);

                int i;
                CalculateRatio(ref t, out i);
                return Bezier.GetPoint(_points[i], _points[i + 1], _points[i + 2], _points[i + 3], t);
            }

            public Vector3 GetVelocity(float t)
            {
                if (_points.Length == 3)
                    return Bezier.GetFirstDerivative(_points[0], _points[1], _points[2], t);

                int i;
                CalculateRatio(ref t, out i);
                return Bezier.GetFirstDerivative(_points[i], _points[i + 1], _points[i + 2], _points[i + 3], t);
            }

            public Vector3 GetDirection(float t)
            {
                return GetVelocity(t).normalized;
            }

            private void CalculateRatio(ref float t, out int i)
            {
                if (t >= 1f)
                {
                    t = 1f;
                    i = _points.Length - 4;
                }
                else
                {
                    t = Mathf.Clamp01(t) * ((_points.Length - 1) / 3);
                    i = (int)t;
                    t -= i;
                    i *= 3;
                }
            }
        }

        #endregion
    }
}
