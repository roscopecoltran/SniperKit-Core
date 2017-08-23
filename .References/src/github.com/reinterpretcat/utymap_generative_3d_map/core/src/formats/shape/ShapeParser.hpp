#ifndef FORMATS_SHAPE_SHAPEPARSER_HPP_INCLUDED
#define FORMATS_SHAPE_SHAPEPARSER_HPP_INCLUDED

#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "formats/FormatTypes.hpp"
#include "utils/CoreUtils.hpp"

#include "shapefile/shapefil.h"

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

namespace utymap {
namespace formats {

template<typename Visitor>
class ShapeParser final {
 public:

  void parse(const std::string &path, Visitor &visitor) const {
    SHPHandle shpFile = SHPOpen(path.c_str(), "rb");
    if (shpFile==NULL)
      throw std::domain_error("Cannot open shp file.");

    int shapeType, entityCount;
    double adfMinBound[4], adfMaxBound[4];
    SHPGetInfo(shpFile, &entityCount, &shapeType, adfMinBound, adfMaxBound);

    DBFHandle dbfFile = DBFOpen(path.c_str(), "rb");
    if (dbfFile==NULL)
      throw std::domain_error("Cannot open dbf file.");

    if (DBFGetFieldCount(dbfFile)==0)
      throw std::domain_error("There are no fields in dbf table.");

    if (entityCount!=DBFGetRecordCount(dbfFile))
      throw std::domain_error("dbf file has different entity count.");

    for (int k = 0; k < entityCount; k++) {
      SHPObject *shape = SHPReadObject(shpFile, k);
      if (shape==NULL)
        throw std::domain_error("Unable to read shape:" + utymap::utils::toString(k));

      Tags tags = parseTags(dbfFile, k);
      visitShape(*shape, tags, visitor);

      SHPDestroyObject(shape);
    }

    DBFClose(dbfFile);
    SHPClose(shpFile);
  }

 private:

  Tags parseTags(DBFHandle dbfFile, int k) const {
    char title[12];
    int fieldCount = DBFGetFieldCount(dbfFile);
    Tags tags;
    tags.reserve(static_cast<std::size_t>(fieldCount));
    for (int i = 0; i < fieldCount; i++) {
      if (DBFIsAttributeNULL(dbfFile, k, i))
        continue;

      utymap::formats::Tag tag;
      int width, decimals;
      DBFFieldType eType = DBFGetFieldInfo(dbfFile, i, title, &width, &decimals);
      tag.key = std::string(title);
      {
        switch (eType) {
          case FTString:tag.value = DBFReadStringAttribute(dbfFile, k, i);
            break;
          case FTInteger:tag.value = utymap::utils::toString(DBFReadIntegerAttribute(dbfFile, k, i));
            break;
          case FTDouble:tag.value = utymap::utils::toString(DBFReadDoubleAttribute(dbfFile, k, i));
            break;
          default:break;
        }
      }
      tags.push_back(tag);
    }
    return std::move(tags);
  }

  void visitShape(const SHPObject &shape, Tags &tags, Visitor &visitor) const {
    switch (shape.nSHPType) {
      case SHPT_POINT:
      case SHPT_POINTM:
      case SHPT_POINTZ:visitPoint(shape, tags, visitor);
        break;
      case SHPT_ARC:
      case SHPT_ARCZ:
      case SHPT_ARCM:visitArc(shape, tags, visitor);
        break;
      case SHPT_POLYGON:
      case SHPT_POLYGONZ:
      case SHPT_POLYGONM:visitPolygon(shape, tags, visitor);
        break;
      case SHPT_MULTIPOINT:
      case SHPT_MULTIPOINTZ:
      case SHPT_MULTIPOINTM:
      case SHPT_MULTIPATCH:std::cerr << "Unsupported shape type:" << SHPTypeName(shape.nSHPType);
        break;
      default:std::cerr << "Unknown shape type:" << SHPTypeName(shape.nSHPType);
        break;
    }
  }

  void visitPoint(const SHPObject &shape, Tags &tags, Visitor &visitor) const {
    utymap::GeoCoordinate coordinate(shape.padfY[0], shape.padfX[0]);
    visitor.visitNode(coordinate, tags);
  }

  void visitArc(const SHPObject &shape, Tags &tags, Visitor &visitor) const {
    if (shape.nParts > 1) {
      std::cerr << "Arc type has more than one part.";
      return;
    }

    std::vector<utymap::GeoCoordinate> coordinates;
    coordinates.reserve(static_cast<std::size_t>(shape.nVertices));
    for (int i = 0; i < shape.nVertices; ++i) {
      coordinates.push_back(utymap::GeoCoordinate(shape.padfY[i], shape.padfX[i]));
    }

    visitor.visitWay(coordinates, tags, coordinates[0]==coordinates[coordinates.size() - 1]);
  }

  void visitPolygon(const SHPObject &shape, Tags &tags, Visitor &visitor) const {
    PolygonMembers members;
    members.reserve(static_cast<std::size_t>(shape.nParts));
    std::size_t coordIndex = 0;
    for (std::size_t i = 0, partNum = 0; i < shape.nVertices; ++i) {
      int startIndex = shape.panPartStart[partNum];
      if (partNum < shape.nParts && startIndex==i) {
        coordIndex = static_cast<std::size_t>(partNum);
        members.push_back(PolygonMember());
        // TODO check inner/outer? Check whether this flag is true for closed polygon only
        members[partNum].isRing = shape.panPartType[partNum]==SHPP_RING;
        int endIndex = partNum==shape.nParts - 1
                       ? shape.nVertices
                       : shape.panPartStart[partNum + 1];
        members[coordIndex].coordinates.reserve(static_cast<std::size_t>(endIndex - startIndex));
        ++partNum;
      }
      members[coordIndex].coordinates.push_back(utymap::GeoCoordinate(shape.padfY[i], shape.padfX[i]));
    }
    visitor.visitRelation(members, tags);
  }
};

}
}

#endif  // FORMATS_SHAPE_SHAPEPARSER_HPP_INCLUDED
