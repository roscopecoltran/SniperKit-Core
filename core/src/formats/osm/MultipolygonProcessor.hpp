#ifndef FORMATS_OSM_MULTIPOLYGONPROCESSOR_HPP_DEFINED
#define FORMATS_OSM_MULTIPOLYGONPROCESSOR_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "formats/FormatTypes.hpp"
#include "formats/osm/OsmDataContext.hpp"

#include <deque>
#include <functional>
#include <memory>

namespace utymap {
namespace formats {

/// Builds relation from multipolygon (see http://wiki.openstreetmap.org/wiki/Talk:Relation:multipolygon)
struct MultipolygonProcessor final {
 private:
  /// Helper class which provides the way to handle coordinate sequences.
  struct CoordinateSequence;
  typedef std::vector<std::shared_ptr<MultipolygonProcessor::CoordinateSequence>> CoordinateSequences;
  typedef utymap::entities::Tag ElementTag;
  typedef std::vector<utymap::entities::Tag> ElementTags;

 public:
  MultipolygonProcessor(utymap::entities::Relation &relation,
                        const utymap::formats::RelationMembers &members,
                        OsmDataContext &context,
                        std::function<void(utymap::entities::Relation &)> resolve);

  /// Builds relation from multipolygon relation.
  void process();

 private:

  void simpleCase(const CoordinateSequences &sequences,
                  const std::vector<int> &outerIndecies,
                  const std::vector<int> &innerIndecies) const;

  void complexCase(CoordinateSequences &sequences) const;

  CoordinateSequences createRings(CoordinateSequences &sequences) const;

  void fillRelation(CoordinateSequences &rings) const;

  static void insertCoordinates(const std::deque<GeoCoordinate> &source,
                                std::vector<GeoCoordinate> &destination,
                                bool isOuter);

  void mergeTags(const ElementTags &tags);

  utymap::entities::Relation &relation_;
  const utymap::formats::RelationMembers &members_;
  utymap::formats::OsmDataContext &context_;
  std::function<void(utymap::entities::Relation &)> resolve_;
};

}
}

#endif // FORMATS_OSM_MULTIPOLYGONPROCESSOR_HPP_DEFINED
