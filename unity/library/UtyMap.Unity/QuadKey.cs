using System;
using System.Text;

namespace UtyMap.Unity
{
    /// <summary> Represents quadkey. </summary>
    public struct QuadKey : IEquatable<QuadKey>
    {
        public readonly int TileX;
        public readonly int TileY;
        public readonly int LevelOfDetail;

        /// <summary> Creates <see cref="QuadKey"/>. </summary>
        public QuadKey(int tileX, int tileY, int levelOfDetail)
        {
            TileX = tileX;
            TileY = tileY;
            LevelOfDetail = levelOfDetail;
        }

        /// <inheritdoc />
        public bool Equals(QuadKey other)
        {
            return TileX == other.TileX && 
                   TileY == other.TileY && 
                   LevelOfDetail == other.LevelOfDetail;
        }

        /// <inheritdoc />
        public override bool Equals(object obj)
        {
            if (!(obj is QuadKey))
                return false;
            return Equals((QuadKey) obj);
        }

        /// <inheritdoc />
        public override int GetHashCode()
        {
            int hash = 17;
            hash = hash * 29 + TileX.GetHashCode();
            hash = hash * 29 + TileY.GetHashCode();
            hash = hash * 29 + LevelOfDetail.GetHashCode();
            return hash;
        }

        /// <inheritdoc />
        public override string ToString()
        {
            var quadKey = new StringBuilder();
            for (var i = LevelOfDetail; i > 0; i--)
            {
                var digit = '0';
                var mask = 1 << (i - 1);
                if ((TileX & mask) != 0)
                    digit++;

                if ((TileY & mask) != 0)
                {
                    digit++;
                    digit++;
                }
                quadKey.Append(digit);
            }
            return quadKey.ToString();
        }

        /// <summary> Returns quadkey from its string representation. </summary>
        /// <exception cref="ArgumentException"> Throws if has invalid digit sequence. </exception>
        public static QuadKey FromString(string value)
        {
            int tileX = 0, tileY = 0;
            int levelOfDetail = value.Length;
            for (int i = levelOfDetail; i > 0; i--)
            {
                int mask = 1 << (i - 1);
                switch (value[levelOfDetail - i])
                {
                    case '0':
                        break;
                    case '1':
                        tileX |= mask;
                        break;
                    case '2':
                        tileY |= mask;
                        break;
                    case '3':
                        tileX |= mask;
                        tileY |= mask;
                        break;

                    default:
                        throw new ArgumentException("Invalid QuadKey digit sequence.");
                }
            }

            return new QuadKey(tileX, tileY, levelOfDetail);
        }
    }
}
