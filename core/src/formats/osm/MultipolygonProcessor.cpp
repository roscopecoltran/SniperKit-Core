#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/osm/MultipolygonProcessor.hpp"

#include "utils/ElementUtils.hpp"
#include "utils/GeoUtils.hpp"
#include "utils/GeometryUtils.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::index;

typedef std::deque<GeoCoordinate> Coords;
typedef std::vector<int> Ints;

struct MultipolygonProcessor::CoordinateSequence final {
  std::uint64_t id;
  Coords coordinates;

  CoordinateSequence(std::uint64_t id, const Coordinates &coordinates) :
      id(id), coordinates(coordinates.begin(), coordinates.end()) {
  }

  /// Tries to add another sequence onto the start or end of this one.
  /// If it succeeds, the other sequence may also be modified and
  /// should be considered "spent".
  bool tryAdd(CoordinateSequence &other) {
    //add the sequence at the end
    if (last()==other.first()) {
      coordinates.pop_back();
      addToEnd(other.coordinates);
      return true;
    }
    //add the sequence backwards at the end
    if (last()==other.last()) {
      coordinates.pop_back();
      other.reverse();
      addToEnd(other.coordinates);
      return true;
    }
    //add the sequence at the beginning
    if (first()==other.last()) {
      coordinates.pop_front();
      addToBegin(other.coordinates);
      return true;
    }
    //add the sequence backwards at the beginning
    if (first()==other.first()) {
      coordinates.pop_front();
      other.reverse();
      addToBegin(other.coordinates);
      return true;
    }
    return false;
  }

  bool isClosed() const {
    return coordinates.size() > 1 && coordinates[0]==coordinates[coordinates.size() - 1];
  }

  bool containsRing(const Coords &other) const {
    return std::all_of(other.begin(), other.end(), [&](const GeoCoordinate &c) {
      return utymap::utils::GeoUtils::isPointInPolygon(c, coordinates.begin(), coordinates.end());
    });
  }

 private:

  GeoCoordinate first() const { return coordinates[0]; }

  GeoCoordinate last() const { return coordinates[coordinates.size() - 1]; }

  void addToBegin(const Coords &other) { coordinates.insert(coordinates.begin(), other.begin(), other.end()); }

  void addToEnd(const Coords &other) { coordinates.insert(coordinates.end(), other.begin(), other.end()); }

  void reverse() { std::reverse(coordinates.begin(), coordinates.end()); }
};

MultipolygonProcessor::MultipolygonProcessor(Relation &relation,
                                             const RelationMembers &members,
                                             OsmDataContext &context,
                                             std::function<void(Relation &)> resolve)
    : relation_(relation), members_(members), context_(context), resolve_(resolve) {
}

/// For algorithm details, see http://wiki.openstreetmap.org/wiki/Relation:multipolygon/Algorithm
void MultipolygonProcessor::process() {
  bool allClosed = true;
  // NOTE do not count multipolygon tag itself
  bool hasNoTags = relation_.tags.size() < 2;
  Ints outerIndecies;
  Ints innerIndecies;
  CoordinateSequences sequences;
  for (const auto &member : members_) {

    if (member.type!="w")
      continue;

    Coordinates coordinates;
    auto wayPair = context_.wayMap.find(member.refId);
    if (wayPair!=context_.wayMap.end()) {
      coordinates = wayPair->second->coordinates;
    } else {
      auto areaPair = context_.areaMap.find(member.refId);
      if (areaPair!=context_.areaMap.end()) {
        coordinates = areaPair->second->coordinates;

        // NOTE make coordinates to be closed ring
        coordinates.push_back(coordinates[0]);

        // NOTE merge tags to relation
        // hasNoTags prevents the case where relation has members with their own tags
        // which should be processed independently (geometry reusage)
        if (member.role=="outer" && hasNoTags)
          mergeTags(areaPair->second->tags);
      } else {
        auto relationPair = context_.relationMap.find(member.refId);
        if (relationPair==context_.relationMap.end())
          return; //  NOTE cannot fill relation: incomplete data

        resolve_(*relationPair->second);
      }
    }

    if (coordinates.empty())
      continue;

    if (member.role=="outer")
      outerIndecies.push_back(static_cast<int>(sequences.size()));
    else if (member.role=="inner")
      innerIndecies.push_back(static_cast<int>(sequences.size()));
    else
      continue;

    auto sequence = std::make_shared<CoordinateSequence>(member.refId, coordinates);
    if (!sequence->isClosed())
      allClosed = false;

    sequences.push_back(sequence);
  }

  if (outerIndecies.size()==1 && allClosed)
    simpleCase(sequences, outerIndecies, innerIndecies);
  else
    complexCase(sequences);
}

void MultipolygonProcessor::simpleCase(const CoordinateSequences &sequences,
                                       const Ints &outerIndecies,
                                       const Ints &innerIndecies) const {
  // TODO set correct tags!
  auto outer = sequences[outerIndecies[0]];

  // outer
  auto outerArea = std::make_shared<Area>();
  outerArea->id = outer->id;
  insertCoordinates(outer->coordinates, outerArea->coordinates, true);

  relation_.elements.push_back(outerArea);

  // inner
  for (int i : innerIndecies) {
    auto innerArea = std::make_shared<Area>();
    insertCoordinates(sequences[i]->coordinates, innerArea->coordinates, false);
    relation_.elements.push_back(innerArea);
  }
}

void MultipolygonProcessor::complexCase(CoordinateSequences &sequences) const {
  CoordinateSequences rings = createRings(sequences);
  if (rings.empty()) return;

  fillRelation(rings);
}

std::vector<std::shared_ptr<MultipolygonProcessor::CoordinateSequence>> MultipolygonProcessor::createRings(
    CoordinateSequences &sequences) const {
  CoordinateSequences closedRings;
  std::shared_ptr<MultipolygonProcessor::CoordinateSequence> currentRing = nullptr;
  while (!sequences.empty()) {
    if (currentRing==nullptr) {
      // start a new ring with any remaining node sequence
      auto lastIndex = sequences.size() - 1;
      currentRing = sequences[lastIndex];
      sequences.erase(sequences.begin() + lastIndex);
    } else {
      // try to continue the ring by appending a node sequence
      bool isFound = false;
      for (auto it = sequences.begin(); it!=sequences.end(); ++it) {
        if (!currentRing->tryAdd(**it)) continue;
        isFound = true;
        sequences.erase(it);
        break;
      }

      if (!isFound)
        return CoordinateSequences();
    }

    // check whether the ring under construction is closed
    if (currentRing!=nullptr && currentRing->isClosed()) {
      // TODO check that it isn't self-intersecting!
      closedRings.push_back(currentRing);
      currentRing = nullptr;
    }
  }

  return std::move(closedRings);
}

void MultipolygonProcessor::fillRelation(CoordinateSequences &rings) const {
  while (!rings.empty()) {
    // find an outer ring
    std::shared_ptr<CoordinateSequence> outer = nullptr;
    for (auto candidate = rings.begin(); candidate!=rings.end(); ++candidate) {
      bool containedInOtherRings = false;
      for (auto other = rings.begin(); other!=rings.end(); ++other) {
        if (other!=candidate && (*other)->containsRing((*candidate)->coordinates)) {
          containedInOtherRings = true;
          break;
        }
      }
      if (containedInOtherRings) continue;
      outer = *candidate;
      rings.erase(candidate);
      break;
    }

    // find inner rings of that ring
    CoordinateSequences inners;
    for (auto ring = rings.begin(); ring!=rings.end();) {
      if (outer->containsRing((*ring)->coordinates)) {
        bool containedInOthers = false;
        for (auto other = rings.begin(); other!=rings.end(); ++other) {
          if (other!=ring && (*other)->containsRing((*ring)->coordinates)) {
            containedInOthers = true;
            break;
          }
        }
        if (!containedInOthers) {
          inners.push_back(*ring);
          ring = rings.erase(ring);
          continue;
        }
      }
      ++ring;
    }

    // outer
    auto outerArea = std::make_shared<Area>();
    outerArea->id = outer->id;
    insertCoordinates(outer->coordinates, outerArea->coordinates, true);
    relation_.elements.push_back(outerArea);

    // inner: create a new area and remove the used rings
    for (const auto &innerRing : inners) {
      auto innerArea = std::make_shared<Area>();
      insertCoordinates(innerRing->coordinates, innerArea->coordinates, false);
      relation_.elements.push_back(innerArea);
    }
  }
}

void MultipolygonProcessor::insertCoordinates(const std::deque<GeoCoordinate> &source,
                                              std::vector<GeoCoordinate> &destination,
                                              bool isOuter) {
  // NOTE we need to remove the last coordinate in area
  std::size_t offset = source[0]==source[source.size() - 1] ? 1 : 0;

  bool isClockwise = utymap::utils::isClockwise(source);
  if ((isOuter && !isClockwise) || (!isOuter && isClockwise))
    destination.insert(destination.end(), source.begin(), source.end() - offset);
  else
    destination.insert(destination.end(), source.rbegin() + offset, source.rend());
}

void MultipolygonProcessor::mergeTags(const ElementTags &tags) {

  auto &dest = relation_.tags;

  dest.insert(dest.end(), tags.cbegin(), tags.cend());

  std::sort(dest.begin(), dest.end());

  dest.erase(std::unique(dest.begin(), dest.end(), [](const ElementTag &l, const ElementTag &r) {
    return l.key==r.key;
  }), dest.end());
}
