using System;
using UnityEngine;
using UtyMap.Unity.Utils;

namespace UtyMap.Unity
{
    /// <summary> Specifies projection functionality. </summary>
    public interface IProjection
    {
        /// <summary> Converts GeoCoordinate to world coordinates. </summary>
        Vector3 Project(GeoCoordinate coordinate, double height);

        /// <summary> Projects world coordinate to geocoordinate. </summary>
        GeoCoordinate Project(Vector3 worldCoordinate);
    }

    /// <summary> Projects coordinates on 2D plane. </summary>
    public class CartesianProjection : IProjection
    {
        private readonly GeoCoordinate _worldZeroPoint;

        /// <summary> Creates instance of <see cref="CartesianProjection"/>. </summary>
        /// <param name="worldZeroPoint"> GeoCoordinate for (0, 0) point. </param>
        public CartesianProjection(GeoCoordinate worldZeroPoint)
        {
            _worldZeroPoint = worldZeroPoint;
        }

        /// <inheritdoc />
        public Vector3 Project(GeoCoordinate coordinate, double height)
        {
            var point = GeoUtils.ToMapCoordinate(_worldZeroPoint, coordinate);
            return new Vector3(point.x, (float) height, point.y);
        }

        /// <inheritdoc />
        public GeoCoordinate Project(Vector3 worldCoordinate)
        {
            return GeoUtils.ToGeoCoordinate(_worldZeroPoint, new Vector2(worldCoordinate.x, worldCoordinate.z));
        }
    }

    /// <summary> Projects coordinates on sphere. </summary>
    public class SphericalProjection : IProjection
    {
        private readonly float _radius;

        /// <summary> Creates instance of <see cref="SphericalProjection"/>. </summary>
        /// <param name="radius"> Radius of sphere. </param>
        public SphericalProjection(float radius)
        {
            _radius = radius;
        }

        /// <inheritdoc />
        public Vector3 Project(GeoCoordinate coordinate, double height)
        {
            // the x-axis goes through long,lat (0,0), so longitude 0 meets the equator;
            // the y-axis goes through (0,90);
            // and the z-axis goes through the pole
            // x = R * cos(lon) * cos(lat)
            // z = R * cos(lat) * sin(lon)
            // y = R * sin(lat)

            double radius = _radius + height;
            double latRad = (Math.PI / 180) * coordinate.Latitude;
            double lonRad = (Math.PI / 180) * coordinate.Longitude;
            float x = (float)(radius * Math.Cos(lonRad) * Math.Cos(latRad));
            float z = (float)(radius * Math.Cos(latRad) * Math.Sin(lonRad));
            float y = (float)(radius * Math.Sin(latRad));

            return new Vector3(x, y, z);
        }

        /// <inheritdoc />
        public GeoCoordinate Project(Vector3 worldCoordinate)
        {
            throw new NotImplementedException();
        }
    }

    /// <summary> Projects coordinates using given projection and scale. </summary>
    public class ScaledProjection : IProjection
    {
        private readonly IProjection _projection;
        private readonly float _scale;
        public ScaledProjection(IProjection projection, float scale)
        {
            _scale = scale;
            _projection = projection;
        }

        /// <inheritdoc />
        public Vector3 Project(GeoCoordinate coordinate, double height)
        {
            return _projection.Project(coordinate, height) * _scale;
        }

        /// <inheritdoc />
        public GeoCoordinate Project(Vector3 worldCoordinate)
        {
            return _projection.Project(worldCoordinate / _scale);
        }
    }
}
