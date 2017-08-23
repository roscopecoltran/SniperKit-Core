#ifndef UTILS_SVGBUILDER_HPP_DEFINED
#define UTILS_SVGBUILDER_HPP_DEFINED

#include "mapcss/Color.hpp"

#include "clipper/clipper.hpp"

#include <cstdint>
#include <cstdarg>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace utymap {
namespace utils {

/// Provides the way to save polygon to svg file which is helpful for debugging.
/// Ported from C# implementation.
class SvgBuilder final {
 public:

  /// Adds path to save.
  void addPaths(const ClipperLib::Paths &poly) {
    if (poly.empty())
      return;

    PolyInfo pi;
    pi.polygons = poly;
    pi.si = style.clone();
    polyInfoList.push_back(pi);
  }

  static void saveToFile(const ClipperLib::Paths &poly,
                         const std::string &filename,
                         double scale = 1.0,
                         int margin = 10) {
    SvgBuilder svgBuilder;
    svgBuilder.addPaths(poly);
    svgBuilder.saveToFile(filename, scale, margin);
  }

  void saveToFile(const std::string &filename, double scale = 1.0, int margin = 10) {
    if (scale==0) scale = 1.0;
    if (margin < 0) margin = 0;

    // calculate the bounding rect ...
    int i = 0, j = 0;
    while (i < polyInfoList.size()) {
      j = 0;
      while (j < polyInfoList[i].polygons.size() && polyInfoList[i].polygons[j].size()==0) j++;
      if (j < polyInfoList[i].polygons.size()) break;
      i++;
    }

    if (i==polyInfoList.size()) return;

    ClipperLib::IntRect rec;
    rec.left = polyInfoList[i].polygons[j][0].X;
    rec.right = rec.left;
    rec.top = polyInfoList[0].polygons[j][0].Y;
    rec.bottom = rec.top;

    for (; i < polyInfoList.size(); i++) {
      for (const auto &pg : polyInfoList[i].polygons)
        for (const auto &pt : pg) {
          if (pt.X < rec.left) rec.left = pt.X;
          else if (pt.X > rec.right) rec.right = pt.X;
          if (pt.Y < rec.top) rec.top = pt.Y;
          else if (pt.Y > rec.bottom) rec.bottom = pt.Y;
        }
    }

    rec.left = static_cast<std::int64_t>(static_cast<double>(rec.left)*scale);
    rec.top = static_cast<std::int64_t>(static_cast<double>(rec.top)*scale);
    rec.right = static_cast<std::int64_t>(static_cast<double>(rec.right)*scale);
    rec.bottom = static_cast<std::int64_t>(static_cast<double>(rec.bottom)*scale);
    std::int64_t offsetX = -rec.left + margin;
    std::int64_t offsetY = -rec.top + margin;

    std::ofstream out(filename);
    out << string_format(svg_header,
                         (rec.right - rec.left) + margin*2,
                         (rec.bottom - rec.top) + margin*2,
                         (rec.right - rec.left) + margin*2,
                         (rec.bottom - rec.top) + margin*2);

    for (const auto &pi : polyInfoList) {
      out << " <path d=\"";
      for (const auto &p : pi.polygons) {
        if (p.size() < 3) continue;

        out << string_format(" M %.2f %.2f",
                             (static_cast<double>(p[0].X)*scale + offsetX),
                             (static_cast<double>(p[0].Y)*scale + offsetY));

        for (int k = 1; k < p.size(); k++) {
          out << string_format(" L %.2f %.2f",
                               (static_cast<double>(p[k].X)*scale + offsetX),
                               (static_cast<double>(p[k].Y)*scale + offsetY));
        }
        out << " z";
      }

      out << string_format(svg_path_format,
                           colorToHex(pi.si.brushClr).c_str(),
                           static_cast<float>(pi.si.brushClr.a)/255,
                           (pi.si.pft==ClipperLib::PolyFillType::pftEvenOdd ? "evenodd" : "nonzero"),
                           colorToHex(pi.si.penClr).c_str(),
                           static_cast<float>(pi.si.penClr.a)/255,
                           pi.si.penWidth);

      if (pi.si.showCoords) {
        out << "<g font-family=\"Verdana\" font-size=\"11\" fill=\"black\">\n\n";
        for (const auto &p : pi.polygons) {
          for (const auto &pt : p) {
            std::int64_t x = pt.X;
            std::int64_t y = pt.Y;
            out << string_format("<text x=\"%d\" y=\"%d\">%d,%d</text>\n",
                                 static_cast<int>(x*scale + offsetX),
                                 static_cast<int>(y*scale + offsetY),
                                 x,
                                 y);
          }
          out << "\n";
        }
        out << "</g>\n";
      }
    }
    out << "</svg>\n";
    out.close();
  }

 private:
  struct StyleInfo {
    ClipperLib::PolyFillType pft;
    utymap::mapcss::Color brushClr;
    utymap::mapcss::Color penClr;
    double penWidth;
    bool showCoords;

    StyleInfo clone() const {
      StyleInfo si;
      si.pft = pft;
      si.brushClr = brushClr;
      si.penClr = penClr;
      si.penWidth = penWidth;
      si.showCoords = showCoords;
      return si;
    }

    StyleInfo() {
      pft = ClipperLib::PolyFillType::pftNonZero;
      brushClr = utymap::mapcss::Color(0xfa, 0xeb, 0xd7, 0xff);
      penClr = utymap::mapcss::Color(0, 0, 0, 0xff);
      penWidth = 0.8;
      showCoords = false;
    }
  };

  struct PolyInfo {
    ClipperLib::Paths polygons;
    StyleInfo si;
  };

  /// TODO rewrite this function
  static std::string string_format(const std::string &fmt, ...) {
    int size = static_cast<int>(fmt.size())*2 + 50;
    std::string str;
    va_list ap;
    while (true) {
      str.resize(size);
      va_start(ap, fmt);
      int n = vsnprintf(const_cast<char *>(str.data()), size, fmt.c_str(), ap);
      va_end(ap);
      if (n > -1 && n < size) {
        str.resize(n);
        return str;
      }
      if (n > -1) size = n + 1;
      else size *= 2;
    }
  }

  static std::string colorToHex(const utymap::mapcss::Color &color) {
    return string_format("#%0x%0x%0x", color.r, color.g, color.b);
  }

  StyleInfo style;
  std::vector<PolyInfo> polyInfoList;
  const std::string svg_header = R"(<?xml version="1.0" standalone="no"?>)" "\n"
      R"(<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.0//EN")" "\n"
      R"("http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd">)" "\n"
      R"(<svg width="%dpx" height="%dpx" viewBox="0 0 %d %d" )"
      R"(version="1.1" xmlns="http://www.w3.org/2000/svg">)" "\n\n";

  const std::string svg_path_format = R"(" style="fill:%s;)"
      R"( fill-opacity:%.2f; fill-rule:%s; stroke:%s;)"
      R"( stroke-opacity:%.2f; stroke-width:%.2f;"/>)" "\n\n";
};

}
}

#endif // UTILS_SVGBUILDER_HPP_DEFINED
