using System;
using System.Collections.Generic;
using UnityEngine;
using UtyMap.Unity.Infrastructure.Primitives;

namespace UtyMap.Unity.Utils
{
    /// <summary> Provides the methods to convert geo coordinates to map coordinates and vice versa. </summary>
    public static class GeoUtils
    {
        #region Coordinate convertion

        /// <summary> The circumference at the equator (latitude 0). </summary>
        private const int LatitudeEquator = 40075160;

        /// <summary> Distance of full circle around the earth through the poles. </summary>
        private const int CircleDistance = 40008000;

        /// <summary>
        ///     Calculates map coordinate from geo coordinate
        ///     see http://stackoverflow.com/questions/3024404/transform-longitude-latitude-into-meters?rq=1
        /// </summary>
        public static Vector2 ToMapCoordinate(GeoCoordinate relativeNullPoint, GeoCoordinate coordinate)
        {
            double deltaLatitude = coordinate.Latitude - relativeNullPoint.Latitude;
            double deltaLongitude = coordinate.Longitude - relativeNullPoint.Longitude;
            double latitudeCircumference = LatitudeEquator*Math.Cos(MathUtils.Deg2Rad(relativeNullPoint.Latitude));
            double resultX = deltaLongitude*latitudeCircumference/360;
            double resultY = deltaLatitude*CircleDistance/360;

            return new Vector2(
                (float)Math.Round(resultX, MathUtils.RoundDigitCount),
                (float)Math.Round(resultY, MathUtils.RoundDigitCount));
        }

        /// <summary> Calculates geo coordinate from map coordinate. </summary>
        public static GeoCoordinate ToGeoCoordinate(GeoCoordinate relativeNullPoint, Vector2 mapPoint)
        {
            return ToGeoCoordinate(relativeNullPoint, mapPoint.x, mapPoint.y);
        }

        /// <summary> Calculates geo coordinate from map coordinate. </summary>
        public static GeoCoordinate ToGeoCoordinate(GeoCoordinate relativeNullPoint, double x, double y)
        {
            double latitudeCircumference = LatitudeEquator*Math.Cos(MathUtils.Deg2Rad(relativeNullPoint.Latitude));

            var deltaLongitude = (x*360)/latitudeCircumference;
            var deltaLatitude = (y*360)/CircleDistance;

            return new GeoCoordinate(relativeNullPoint.Latitude + deltaLatitude,
                relativeNullPoint.Longitude + deltaLongitude);
        }

        #endregion

        #region Distance specific

        // Semi-axes of WGS-84 geoidal reference
        private const double WGS84_a = 6378137.0; // Major semiaxis [m]
        private const double WGS84_b = 6356752.3; // Minor semiaxis [m]

        /// <summary> Calculates distance between two coordinates. </summary>
        /// <param name="first">First coordinate.</param>
        /// <param name="second">Second coordinate.</param>
        /// <returns>Distance.</returns>
        public static double Distance(GeoCoordinate first, GeoCoordinate second)
        {
            var dLat = MathUtils.Deg2Rad((first.Latitude - second.Latitude));
            var dLon = MathUtils.Deg2Rad((first.Longitude - second.Longitude));

            var lat1 = MathUtils.Deg2Rad(first.Latitude);
            var lat2 = MathUtils.Deg2Rad(second.Latitude);

            var a = Math.Sin(dLat / 2) * Math.Sin(dLat / 2) +
                    Math.Sin(dLon / 2) * Math.Sin(dLon / 2) * Math.Cos(lat1) * Math.Cos(lat2);

            var c = 2 * Math.Atan2(Math.Sqrt(a), Math.Sqrt(1 - a));

            var radius = WGS84EarthRadius(dLat);

            return radius * c;
        }

        /// <summary> Earth radius at a given latitude, according to the WGS-84 ellipsoid [m]. </summary>
        public static double WGS84EarthRadius(double lat)
        {
            // http://en.wikipedia.org/wiki/Earth_radius
            var an = WGS84_a * WGS84_a * Math.Cos(lat);
            var bn = WGS84_b * WGS84_b * Math.Sin(lat);
            var ad = WGS84_a * Math.Cos(lat);
            var bd = WGS84_b * Math.Sin(lat);
            return Math.Sqrt((an * an + bn * bn) / (ad * ad + bd * bd));
        }

        #endregion

        #region Factory methods

        public static QuadKey CreateQuadKey(GeoCoordinate coordinate, int levelOfDetail)
        {
            return new QuadKey(
                LonToTileX(Clip(coordinate.Longitude, -179.9999999, 179.9999999), levelOfDetail),
                LatToTileY(Clip(coordinate.Latitude, -85.05112877, 85.05112877), levelOfDetail),
                levelOfDetail);
        }

        public static BoundingBox QuadKeyToBoundingBox(QuadKey quadKey)
        {
            int levelOfDetail = quadKey.LevelOfDetail;

            var minPoint = new GeoCoordinate(
                 TileYToLat(quadKey.TileY + 1, levelOfDetail),
                 TileXToLon(quadKey.TileX, levelOfDetail));

            var maxPoint = new GeoCoordinate(
                 TileYToLat(quadKey.TileY, levelOfDetail),
                 TileXToLon(quadKey.TileX + 1, levelOfDetail));

            return new BoundingBox(minPoint, maxPoint);
        }

        public static Rectangle QuadKeyToRect(IProjection projection, QuadKey quadKey)
        {
            var boundingBox = QuadKeyToBoundingBox(quadKey);
            var minPoint = projection.Project(boundingBox.MinPoint, 0);
            var maxPoint = projection.Project(boundingBox.MaxPoint, 0);

            return new Rectangle(minPoint.x, minPoint.z, maxPoint.x - minPoint.x, maxPoint.z - minPoint.z);
        }

        public static BoundingBox RectToBoundingBox(IProjection projection, Rectangle rectangle)
        {
            var minPoint = projection.Project(new Vector3(rectangle.BottomLeft.x, 0, rectangle.BottomLeft.y));
            var maxPoint = projection.Project(new Vector3(rectangle.TopRight.x, 0, rectangle.TopRight.y));

            return new BoundingBox(minPoint, maxPoint);
        }

        public static IEnumerable<QuadKey> BoundingBoxToQuadKeys(BoundingBox bbox, int levelOfDetails)
        {
            var minQuadKey = CreateQuadKey(bbox.MinPoint, levelOfDetails);
            var maxQuadKey = CreateQuadKey(bbox.MaxPoint, levelOfDetails);

            var width = maxQuadKey.TileX - minQuadKey.TileX + 1;
            var height = minQuadKey.TileY - maxQuadKey.TileY + 1;

            var halfWidth = width/2;
            var halfHeight = height/2;

            var xBase = minQuadKey.TileX + halfWidth;
            var yBase = maxQuadKey.TileY + halfHeight;

            int x = 0, y = 0, dx = 0, dy = -1;
            int t = Math.Max(width, height);
            int maxI = t*t;
            for (int i = 0; i < maxI; i++)
            {
                if ((-halfWidth <= x) && (x <= halfWidth) && (-halfHeight <= y) && (y <= halfHeight))
                    yield return new QuadKey(xBase + x, yBase - y, levelOfDetails);

                if ((x == y) || ((x < 0) && (x == -y)) || ((x > 0) && (x == 1 - y)))
                {
                    t = dx;
                    dx = -dy;
                    dy = t;
                }
                x += dx;
                y += dy;
            }
        }

        private static int LonToTileX(double longitude, int levelOfDetail)
        {
            return (int)(Math.Floor((longitude + 180.0) / 360.0 * Math.Pow(2.0, levelOfDetail)));
        }

        private static int LatToTileY(double latitude, int levelOfDetail)
        {
            return (int)(Math.Floor((1.0 - Math.Log(Math.Tan(latitude * Math.PI / 180.0) +
                                                     1.0 / Math.Cos(latitude * Math.PI / 180.0)) / Math.PI) / 2.0 *
                                     Math.Pow(2.0, levelOfDetail)));
        }

         private static double TileXToLon(int x, int levelOfDetail)
    {
        return x / Math.Pow(2.0, levelOfDetail) * 360.0 - 180;
    }

        private static double TileYToLat(int y, int levelOfDetail)
        {
            double n = Math.PI - 2.0 * Math.PI * y / Math.Pow(2.0, levelOfDetail);
            return 180.0 / Math.PI * Math.Atan(0.5 * (Math.Exp(n) - Math.Exp(-n)));
        }

        private static double Clip(double n, double minValue, double maxValue)
        {
            return Math.Max(minValue, Math.Min(n, maxValue));
        }

        #endregion
    }
}