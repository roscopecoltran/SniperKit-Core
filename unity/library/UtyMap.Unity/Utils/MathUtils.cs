using System;

namespace UtyMap.Unity.Utils
{
    internal static class MathUtils
    {
        /// <summary> Round digit count. </summary>
        internal const int RoundDigitCount = 5;

        /// <summary> Converts degrees to radians. </summary>
        public static double Deg2Rad(double degrees)
        {
            return Math.PI*degrees/180.0;
        }

        /// <summary> Converts radians to degrees. </summary>
        public static double Rad2Deg(double radians)
        {
            return 180.0*radians/Math.PI;
        }

        /// <summary> Compares equality of two double using epsilon. </summary>
        public static bool AreEqual(double a, double b, double epsilon = double.Epsilon)
        {
            return Math.Abs(a - b) < epsilon;
        }
    }
}