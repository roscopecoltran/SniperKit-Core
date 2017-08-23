#ifndef FORMATS_JSON_OSMJSONPARSER_HPP_INCLUDED
#define FORMATS_JSON_OSMJSONPARSER_HPP_INCLUDED

#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "index/StringTable.hpp"
#include "utils/ElementUtils.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace utymap {
namespace formats {

template<typename Visitor>
class OsmJsonParser {
  const std::string IdAttributeName = "id";
  const std::string FeatureAttributeName = "feature";
  using ptree = boost::property_tree::ptree;

  struct FoldRelation final : public utymap::entities::ElementVisitor {
    std::shared_ptr<utymap::entities::Element> element;

    void visitNode(const utymap::entities::Node &n) override {
      element = std::make_shared<utymap::entities::Node>(n);
    }

    void visitWay(const utymap::entities::Way &w) override {
      element = std::make_shared<utymap::entities::Way>(w);
    }

    void visitArea(const utymap::entities::Area &a) override {
      element = std::make_shared<utymap::entities::Area>(a);
    }

    void visitRelation(const utymap::entities::Relation &r) override {
      element = std::make_shared<utymap::entities::Relation>(r);
    }
  };

 public:

  OsmJsonParser(const utymap::index::StringTable &stringTable) :
      stringTable_(stringTable),
      idKey_(stringTable.getId(IdAttributeName)),
      featureKey_(stringTable.getId(FeatureAttributeName)) {
  }

  /// Parses osm json data from stream calling visitor.
  void parse(std::istream &istream, Visitor &visitor) const {
    ptree pt;
    read_json(istream, pt);

    for (const ptree::value_type &feature : pt) {
      std::string featureName = feature.first;
      std::uint32_t featureId = stringTable_.getId(featureName);
      for (const ptree::value_type &f : pt.get_child(featureName).get_child("features")) {
        const auto &type = f.second.get_child("geometry.type").data();
        if (type=="Point")
          parsePoint(visitor, featureId, f.second);
        else if (type=="LineString")
          parseLineString(visitor, featureId, f.second);
        else if (type=="Polygon")
          parsePolygon(visitor, featureId, f.second);
        else if (type=="MultiLineString")
          parseMultiLineString(visitor, featureId, f.second);
        else if (type=="MultiPolygon")
          parseMultiPolygon(visitor, featureId, f.second);
        else
          throw std::invalid_argument(std::string("Unknown geometry type:") + type);
      }
    }
  }

 private:

  /// Parses relation with relations from multipolygon and notifies visitor.
  void parseMultiPolygon(Visitor &visitor, std::uint32_t featureId, const ptree &feature) const {
    utymap::entities::Relation relation;
    parseProperties(relation, featureId, feature.get_child("properties"));
    for (const ptree::value_type &geometry : feature.get_child("geometry.coordinates")) {
      auto child = parseRelation(featureId, geometry.second);
      if (child.elements.size()==1) {
        FoldRelation fold;
        child.elements[0]->accept(fold);
        relation.elements.push_back(fold.element);
      } else {
        relation.elements.push_back(std::make_shared<utymap::entities::Relation>(child));
      }
    }
    visitor.add(relation);
  }

  /// Parses relation with areas from polygon (first is outer, nexts are inner) and notifies visitor.
  void parsePolygon(Visitor &visitor, std::uint32_t featureId, const ptree &feature) const {
    processSimpleRelation(visitor, featureId, feature);
  }

  /// Parses relation with ways from multiline string and notifies visitor.
  void parseMultiLineString(Visitor &visitor, std::uint32_t featureId, const ptree &feature) const {
    processSimpleRelation(visitor, featureId, feature);
  }

  /// Parses way from line string and notifies visitor.
  void parseLineString(Visitor &visitor, std::uint32_t featureId, const ptree &feature) const {
    utymap::entities::Way way;
    parseProperties(way, featureId, feature.get_child("properties"));
    way.coordinates = parseCoordinates(feature.get_child("geometry.coordinates"));
    visitor.add(way);
  }

  /// Parses node from point and notifies visitor.
  void parsePoint(Visitor &visitor, std::uint32_t featureId, const ptree &feature) const {
    utymap::entities::Node node;
    parseProperties(node, featureId, feature.get_child("properties"));
    node.coordinate = parseCoordinate(feature.get_child("geometry.coordinates"));
    visitor.add(node);
  }

  /// Parses relation as simple relation with non-relation children. If child is single, then
  /// calls visitor with this child instead of relation.
  void processSimpleRelation(Visitor &visitor, std::uint32_t featureId, const ptree &feature) const {
    auto relation = parseRelation(featureId, feature.get_child("geometry.coordinates"));
    if (relation.elements.size()==1) {
      parseProperties(*relation.elements[0], featureId, feature.get_child("properties"));
      visitor.add(*relation.elements[0]);
    } else {
      parseProperties(relation, featureId, feature.get_child("properties"));
      visitor.add(relation);
    }
  }

  /// Returns parsed relation.
  utymap::entities::Relation parseRelation(std::uint32_t featureId, const ptree &geometry) const {
    utymap::entities::Relation relation;
    relation.id = 0;
    for (const ptree::value_type &coordTree : geometry) {
      auto coordinates = parseCoordinates(coordTree.second);
      if (coordinates.size() > 3 && coordinates[0]==coordinates[coordinates.size() - 1])
        addToRelation<utymap::entities::Area>(relation, coordinates);
      else
        addToRelation<utymap::entities::Way>(relation, coordinates);
    }
    return relation;
  }

  template<typename T>
  void addToRelation(utymap::entities::Relation &relation, std::vector<utymap::GeoCoordinate> &coordinates) const {
    auto element = std::make_shared<T>();
    element->id = 0;
    element->coordinates = std::move(coordinates);
    relation.elements.push_back(element);
  }

  /// Parses coordinates list. If input data is invalid, return false.
  static std::vector<utymap::GeoCoordinate> parseCoordinates(const ptree &geometry) {
    std::vector<utymap::GeoCoordinate> coordinates;
    for (const ptree::value_type &coordinate : geometry) {
      coordinates.emplace_back(parseCoordinate(coordinate.second));
    }

    // TODO check orientation
    std::reverse(coordinates.begin(), coordinates.end());
    return coordinates;
  }

  static utymap::GeoCoordinate parseCoordinate(const ptree &coordinate) {
    if (std::distance(coordinate.begin(), coordinate.end())!=2)
      throw std::invalid_argument("Invalid geometry.");

    auto iter = coordinate.begin();
    double longitude = iter->second.get_value<double>();
    double latitude = (++iter)->second.get_value<double>();
    return utymap::GeoCoordinate(latitude, longitude);
  }

  void parseProperties(utymap::entities::Element &element, std::uint32_t featureId, const ptree &properties) const {
    element.id = 0;
    for (const ptree::value_type &property : properties) {
      std::uint32_t key = stringTable_.getId(property.first);
      if (key==idKey_)
        element.id = parseId(property.second.data());
      else
        element.tags.emplace_back(key, stringTable_.getId(property.second.data()));
    }

    // NOTE add artificial tag for mapcss processing.
    element.tags.emplace_back(featureKey_, featureId);

    std::sort(element.tags.begin(), element.tags.end());
  }

  std::uint64_t parseId(const std::string &value) const {
    // NOTE ID can be negative for relations:
    // see http://wiki.openstreetmap.org/wiki/Osm2pgsql/schema#Processed_Data
    if (!value.empty()) {
      return utymap::utils::lexicalCast<std::uint64_t>(value[0]=='-'
                                                       ? value.substr(1, value.size() - 1)
                                                       : value);
    }
    return 0;
  }

  const utymap::index::StringTable &stringTable_;
  const std::uint32_t idKey_;
  const std::uint32_t featureKey_;
};

}
}

#endif  // FORMATS_JSON_OSMJSONPARSER_HPP_INCLUDED