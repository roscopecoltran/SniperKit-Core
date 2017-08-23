#include "utils/NoiseUtils.hpp"

using namespace utymap::math;
using namespace utymap::utils;

const double Sqr2 = std::sqrt(2);

const int NoiseUtils::Hash[] =
    {
        151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
        140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
        247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
        57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
        74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
        60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
        65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
        200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
        52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
        207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
        119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
        129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
        218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
        81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
        184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
        222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,
        151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
        140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
        247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
        57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
        74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
        60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
        65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
        200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
        52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
        207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
        119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
        129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
        218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
        81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
        184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
        222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
    };

const Vector2 NoiseUtils::Gradients2D[] =
    {
        Vector2(1, 0),
        Vector2(-1, 0),
        Vector2(0, 1),
        Vector2(0, -1),
        Vector2(1/Sqr2, 1/Sqr2),
        Vector2(-1/Sqr2, 1/Sqr2),
        Vector2(1/Sqr2, -1/Sqr2),
        Vector2(-1/Sqr2, -1/Sqr2)
    };

const Vector3 NoiseUtils::Gradients3D[] =
    {
        Vector3(1, 1, 0),
        Vector3(-1, 1, 0),
        Vector3(1, -1, 0),
        Vector3(-1, -1, 0),
        Vector3(1, 0, 1),
        Vector3(-1, 0, 1),
        Vector3(1, 0, -1),
        Vector3(-1, 0, -1),
        Vector3(0, 1, 1),
        Vector3(0, -1, 1),
        Vector3(0, 1, -1),
        Vector3(0, -1, -1),
        Vector3(1, 1, 0),
        Vector3(-1, 1, 0),
        Vector3(0, -1, 1),
        Vector3(0, -1, -1)
    };

double NoiseUtils::perlin2D(double x, double y, double frequency) {
  if (frequency < 1E-5) return 0;

  Vector2 point(x*frequency, y*frequency);

  int ix0 = static_cast<int>(std::floor(point.x));
  int iy0 = static_cast<int>(std::floor(point.y));
  double tx0 = point.x - ix0;
  double ty0 = point.y - iy0;
  double tx1 = tx0 - 1;
  double ty1 = ty0 - 1;
  ix0 &= HashMask;
  iy0 &= HashMask;
  int ix1 = ix0 + 1;
  int iy1 = iy0 + 1;

  int h0 = Hash[ix0];
  int h1 = Hash[ix1];
  const Vector2 &g00 = Gradients2D[Hash[h0 + iy0] & GradientsMask2D];
  const Vector2 &g10 = Gradients2D[Hash[h1 + iy0] & GradientsMask2D];
  const Vector2 &g01 = Gradients2D[Hash[h0 + iy1] & GradientsMask2D];
  const Vector2 &g11 = Gradients2D[Hash[h1 + iy1] & GradientsMask2D];

  double v00 = dot(g00, tx0, ty0);
  double v10 = dot(g10, tx1, ty0);
  double v01 = dot(g01, tx0, ty1);
  double v11 = dot(g11, tx1, ty1);

  double tx = smooth(tx0);
  double ty = smooth(ty0);

  double a = v00;
  double b = v10 - v00;
  double c = v01 - v00;
  double d = v11 - v01 - v10 + v00;

  return (a + b*tx + (c + d*tx)*ty)*Sqr2;
}

double NoiseUtils::perlin3D(double x, double y, double z, double frequency) {
  if (frequency < 1E-5) return 0;

  Vector3 point(x*frequency, y*frequency, z*frequency);
  int ix0 = static_cast<int>(std::floor(point.x));
  int iy0 = static_cast<int>(std::floor(point.y));
  int iz0 = static_cast<int>(std::floor(point.z));
  double tx0 = point.x - ix0;
  double ty0 = point.y - iy0;
  double tz0 = point.z - iz0;
  double tx1 = tx0 - 1;
  double ty1 = ty0 - 1;
  double tz1 = tz0 - 1;
  ix0 &= HashMask;
  iy0 &= HashMask;
  iz0 &= HashMask;
  int ix1 = ix0 + 1;
  int iy1 = iy0 + 1;
  int iz1 = iz0 + 1;

  int h0 = Hash[ix0];
  int h1 = Hash[ix1];
  int h00 = Hash[h0 + iy0];
  int h10 = Hash[h1 + iy0];
  int h01 = Hash[h0 + iy1];
  int h11 = Hash[h1 + iy1];
  const Vector3 &g000 = Gradients3D[Hash[h00 + iz0] & GradientsMask3D];
  const Vector3 &g100 = Gradients3D[Hash[h10 + iz0] & GradientsMask3D];
  const Vector3 &g010 = Gradients3D[Hash[h01 + iz0] & GradientsMask3D];
  const Vector3 &g110 = Gradients3D[Hash[h11 + iz0] & GradientsMask3D];
  const Vector3 &g001 = Gradients3D[Hash[h00 + iz1] & GradientsMask3D];
  const Vector3 &g101 = Gradients3D[Hash[h10 + iz1] & GradientsMask3D];
  const Vector3 &g011 = Gradients3D[Hash[h01 + iz1] & GradientsMask3D];
  const Vector3 &g111 = Gradients3D[Hash[h11 + iz1] & GradientsMask3D];

  double v000 = dot(g000, tx0, ty0, tz0);
  double v100 = dot(g100, tx1, ty0, tz0);
  double v010 = dot(g010, tx0, ty1, tz0);
  double v110 = dot(g110, tx1, ty1, tz0);
  double v001 = dot(g001, tx0, ty0, tz1);
  double v101 = dot(g101, tx1, ty0, tz1);
  double v011 = dot(g011, tx0, ty1, tz1);
  double v111 = dot(g111, tx1, ty1, tz1);

  double tx = smooth(tx0);
  double ty = smooth(ty0);
  double tz = smooth(tz0);

  double a = v000;
  double b = v100 - v000;
  double c = v010 - v000;
  double d = v001 - v000;
  double e = v110 - v010 - v100 + v000;
  double f = v101 - v001 - v100 + v000;
  double g = v011 - v001 - v010 + v000;
  double h = v111 - v011 - v101 + v001 - v110 + v010 + v100 - v000;

  return a + b*tx + (c + e*tx)*ty + (d + f*tx + (g + h*tx)*ty)*tz;
}
