using System;
using UnityEngine;

namespace UtyMap.Unity.Infrastructure.Primitives
{
    /// <summary> Represents rectangle in 2D space. </summary>
    public struct Rectangle
    {
        private readonly float _xmin;
        private readonly float _ymin;
        private readonly float _xmax;
        private readonly float _ymax;

        /// <summary> Initializes a new instance of the <see cref="Rectangle"/> class with predefined bounds. </summary>
        /// <param name="x"> Minimum x value (left). </param>
        /// <param name="y"> Minimum y value (bottom). </param>
        /// <param name="width"> Width of the rectangle. </param>
        /// <param name="height"> Height of the rectangle. </param>
        public Rectangle(float x, float y, float width, float height)
        {
            _xmin = x;
            _ymin = y;
            _xmax = x + width;
            _ymax = y + height;
        }

        /// <summary> Gets left. </summary>
        public float Left { get { return _xmin; } }

        /// <summary> Gets right. </summary>
        public float Right { get { return _xmax; } }

        /// <summary> Gets bottom. </summary>
        public float Bottom { get { return _ymin; } }

        /// <summary> Gets top. </summary>
        public float Top { get { return _ymax; } }

        /// <summary> Gets left bottom point. </summary>
        public Vector2 BottomLeft { get { return new Vector2(_xmin, _ymin); } }

        /// <summary> Gets right top point. </summary>
        public Vector2 TopRight { get { return new Vector2(_xmax, _ymax); } }

        /// <summary> Gets left top point. </summary>
        public Vector2 TopLeft { get { return new Vector2(_xmin, _ymax); } }

        /// <summary> Gets right bottom point. </summary>
        public Vector2 BottomRight { get { return new Vector2(_xmax, _ymin); } }

        /// <summary> Gets center point. </summary>
        public Vector2 Center { get { return new Vector2(_xmin + Width/2, _ymin + Height/2); } }

        /// <summary> Gets the width of the bounding box. </summary>
        public float Width { get { return _xmax - _xmin; } }

        /// <summary> Gets the height of the bounding box. </summary>
        public float Height { get { return _ymax - _ymin; } }

        /// <inheritdoc />
        public override string ToString()
        {
            return String.Format("[{0},{1}]", BottomLeft, TopRight);
        }
    }
}
