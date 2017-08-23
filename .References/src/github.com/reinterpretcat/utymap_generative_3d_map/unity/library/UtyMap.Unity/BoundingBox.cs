﻿using System;
using UtyMap.Unity.Utils;

namespace UtyMap.Unity
{
    /// <summary> Represents geo bounding box. </summary>
    public class BoundingBox
    {
        /// <summary> Point with maximum latitude and longitude. </summary>
        public GeoCoordinate MaxPoint;

        /// <summary> Point with minimal latitude and longitude. </summary>
        public GeoCoordinate MinPoint;

        /// <summary> Creates invalid bounding box which can be modified later (e.g. by calling Extend). </summary>
        public BoundingBox()
            : this(new GeoCoordinate(90, 180), new GeoCoordinate(-90, -180))
        {
        }

        /// <summary> Creates bounding box from given min and max points. </summary>
        /// <param name="minPoint">Point with minimal latitude and longitude</param>
        /// <param name="maxPoint">Point with maximum latitude and longitude</param>
        public BoundingBox(GeoCoordinate minPoint, GeoCoordinate maxPoint)
        {
            MinPoint = minPoint;
            MaxPoint = maxPoint;
        }

        /// <summary> Extends current bounding box by given coordinate. </summary>
        public void Extend(GeoCoordinate coordinate)
        {
            MinPoint = new GeoCoordinate(
                Math.Min(MinPoint.Latitude, coordinate.Latitude),
                Math.Min(MinPoint.Longitude, coordinate.Longitude));

            MaxPoint = new GeoCoordinate(
                Math.Max(MaxPoint.Latitude, coordinate.Latitude),
                Math.Max(MaxPoint.Longitude, coordinate.Longitude));
        }

        /// <summary> Returns center of bounding box. </summary>
        public GeoCoordinate Center()
        {
            return new GeoCoordinate(
                MinPoint.Latitude + (MaxPoint.Latitude - MinPoint.Latitude) / 2,
                MinPoint.Longitude + (MaxPoint.Longitude - MinPoint.Longitude) / 2);
        }

        /// <inheritdoc />
        public override string ToString()
        {
            return string.Format("{0},{1}", MinPoint, MaxPoint);
        }

        #region Static operations

        /// <summary> Adds geo coordinate to a bounding box. </summary>
        public static BoundingBox operator +(BoundingBox a, GeoCoordinate b)
        {
            a.MinPoint = new GeoCoordinate(
                a.MinPoint.Latitude < b.Latitude ? a.MinPoint.Latitude : b.Latitude,
                a.MinPoint.Longitude < b.Longitude ? a.MinPoint.Longitude : b.Longitude);

            a.MaxPoint = new GeoCoordinate(
                a.MaxPoint.Latitude > b.Latitude ? a.MaxPoint.Latitude : b.Latitude,
                a.MaxPoint.Longitude > b.Longitude ? a.MaxPoint.Longitude : b.Longitude);

            return a;
        }

        /// <summary> Adds the right bounding box to the left one. </summary>
        public static BoundingBox operator +(BoundingBox a, BoundingBox b)
        {
            var minLat = a.MinPoint.Latitude < b.MinPoint.Latitude ? a.MinPoint.Latitude : b.MinPoint.Latitude;
            var minLon = a.MinPoint.Longitude < b.MinPoint.Longitude ? a.MinPoint.Longitude : b.MinPoint.Longitude;

            var maxLat = a.MaxPoint.Latitude > b.MaxPoint.Latitude ? a.MaxPoint.Latitude : b.MaxPoint.Latitude;
            var maxLon = a.MaxPoint.Longitude > b.MaxPoint.Longitude ? a.MaxPoint.Longitude : b.MaxPoint.Longitude;

            a.MinPoint = new GeoCoordinate(minLat, minLon);
            a.MaxPoint = new GeoCoordinate(maxLat, maxLon);

            return a;
        }

        #endregion

        # region Creation

        /// <summary> Creates bounding box as rectangle of given size. </summary>
        /// <param name="center">Center point.</param>
        /// <param name="width">Width in meters.</param>
        /// <param name="height">Heigh in meters.</param>
        public static BoundingBox Create(GeoCoordinate center, double width, double height)
        {
            // Bounding box surrounding the point at given coordinates,
            // assuming local approximation of Earth surface as a sphere
            // of radius given by WGS84
            var lat = MathUtils.Deg2Rad(center.Latitude);
            var lon = MathUtils.Deg2Rad(center.Longitude);

            // Radius of Earth at given latitude
            var radius = GeoUtils.WGS84EarthRadius(lat);
            // Radius of the parallel at given latitude
            var pradius = radius*Math.Cos(lat);

            var dWidth = width/(2*radius);
            var dHeight = height/(2*pradius);

            var latMin = lat - dWidth;
            var latMax = lat + dWidth;
            var lonMin = lon - dHeight;
            var lonMax = lon + dHeight;

            return new BoundingBox(
                new GeoCoordinate(MathUtils.Rad2Deg(latMin), MathUtils.Rad2Deg(lonMin)),
                new GeoCoordinate(MathUtils.Rad2Deg(latMax), MathUtils.Rad2Deg(lonMax)));
        }

        /// <summary> Creates bounding box as square of given size. </summary>
        /// <param name="center">Center.</param>
        /// <param name="sideInMeters">Side length in meters.</param>
        public static BoundingBox Create(GeoCoordinate center, float sideInMeters)
        {
            return Create(center, sideInMeters, sideInMeters);
        }

        /// <summary> Creates empty bounding box. </summary>
        public static BoundingBox Empty()
        {
            return new BoundingBox(new GeoCoordinate(double.MaxValue, double.MaxValue),
                new GeoCoordinate(double.MinValue, double.MinValue));
        }

        #endregion

        #region Test functions

        /// <summary> Checks whether given coordinate inside bounding box. </summary>
        public bool Contains(GeoCoordinate coordinate)
        {
            return coordinate.Latitude > MinPoint.Latitude && coordinate.Longitude > MinPoint.Longitude &&
                   coordinate.Latitude < MaxPoint.Latitude && coordinate.Longitude < MaxPoint.Longitude;
        }

        /// <summary> Checks whether bounding boxes are intersects </summary>
        public bool Intersects(BoundingBox bbox)
        {
            var minX = Math.Max(MinPoint.Latitude, bbox.MinPoint.Latitude);
            var minY = Math.Max(MinPoint.Longitude, bbox.MinPoint.Longitude);
            var maxX = Math.Min(MaxPoint.Latitude, bbox.MaxPoint.Latitude);
            var maxY = Math.Min(MaxPoint.Longitude, bbox.MaxPoint.Longitude);

            return minX <= maxX && minY <= maxY;
        }

        #endregion
    }
}
