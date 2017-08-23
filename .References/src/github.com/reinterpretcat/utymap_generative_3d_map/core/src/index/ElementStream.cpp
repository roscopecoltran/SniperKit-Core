#include "GeoCoordinate.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "index/ElementStream.hpp"
#include "utils/CoreUtils.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::index;

namespace {

const char NodeType = 0;
const char WayType = 1;
const char AreaType = 2;
const char RelationType = 3;

std::ostream &operator<<(std::ostream &stream, const Tag &tag) {
  stream.write(reinterpret_cast<const char *>(&tag.key), sizeof(tag.key));
  stream.write(reinterpret_cast<const char *>(&tag.value), sizeof(tag.value));
  return stream;
}

std::istream &operator>>(std::istream &stream, Tag &tag) {
  stream.read(reinterpret_cast<char *>(&tag.key), sizeof(tag.key));
  stream.read(reinterpret_cast<char *>(&tag.value), sizeof(tag.value));
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const utymap::GeoCoordinate &coordinate) {
  stream.write(reinterpret_cast<const char *>(&coordinate.latitude), sizeof(coordinate.latitude));
  stream.write(reinterpret_cast<const char *>(&coordinate.longitude), sizeof(coordinate.longitude));
  return stream;
}

std::istream &operator>>(std::istream &stream, utymap::GeoCoordinate &coordinate) {
  stream.read(reinterpret_cast<char *>(&coordinate.latitude), sizeof(coordinate.latitude));
  stream.read(reinterpret_cast<char *>(&coordinate.longitude), sizeof(coordinate.longitude));
  return stream;
}

template<typename T>
std::ostream &operator<<(std::ostream &stream, const std::vector<T> &data) {
  std::uint16_t size = static_cast<std::uint16_t>(data.size());
  stream.write(reinterpret_cast<const char *>(&size), sizeof(size));
  for (const auto &item : data)
    stream << item;
  return stream;
}

template<typename T>
std::istream &operator>>(std::istream &stream, std::vector<T> &data) {
  std::uint16_t size = 0;
  stream.read(reinterpret_cast<char *>(&size), sizeof(size));
  data.resize(size);
  for (size_t i = 0; i < size; ++i)
    stream >> data[i];
  return stream;
}

/// Writes element to stream.
struct ElementWriter : ElementVisitor {
  explicit ElementWriter(std::ostream &s) : stream_(s) {}

  void visitNode(const Node &node) override {
    stream_ << NodeType << node.tags << node.coordinate;
  }

  void visitWay(const Way &way) override {
    stream_ << WayType << way.tags << way.coordinates;
  }

  void visitArea(const Area &area) override {
    stream_ << AreaType << area.tags << area.coordinates;
  }

  void visitRelation(const Relation &relation) override {
    stream_ << RelationType << relation.tags;
    auto size = static_cast<std::uint16_t>(relation.elements.size());
    stream_.write(reinterpret_cast<const char *>(&size), sizeof(size));
    for (const auto &element : relation.elements) {
      stream_.write(reinterpret_cast<const char *>(&element->id), sizeof(element->id));
      element->accept(*this);
    }
  }

  std::ostream &stream_;
};

/// Reads element from stream.
class ElementReader final {
 public:
  explicit ElementReader(std::istream &stream) : stream_(stream) {
  }

  std::unique_ptr<Element> read() const {
    char elementType;
    stream_ >> elementType;

    switch (elementType) {
      case NodeType:return readNode();
      case WayType:return readWay();
      case AreaType:return readArea();
      case RelationType:return readRelation();
      default:throw std::domain_error("Unknown element type.");
    }
  }

 private:
  std::unique_ptr<Node> readNode() const {
    auto node = utymap::utils::make_unique<Node>();
    stream_ >> node->tags >> node->coordinate;
    return std::move(node);
  }

  std::unique_ptr<Way> readWay() const {
    auto way = utymap::utils::make_unique<Way>();
    stream_ >> way->tags >> way->coordinates;
    return std::move(way);
  }

  std::unique_ptr<Area> readArea() const {
    auto area = utymap::utils::make_unique<Area>();
    stream_ >> area->tags >> area->coordinates;
    return std::move(area);
  }

  std::unique_ptr<Relation> readRelation() const {
    auto relation = utymap::utils::make_unique<Relation>();
    stream_ >> relation->tags;

    std::uint16_t elementSize = 0;
    stream_.read(reinterpret_cast<char *>(&elementSize), sizeof(elementSize));

    for (std::uint16_t i = 0; i < elementSize; ++i) {
      std::uint64_t id;
      stream_.read(reinterpret_cast<char *>(&id), sizeof(id));
      auto element = read();
      element->id = id;
      relation->elements.push_back(std::move(element));
    }
    return relation;
  }

  std::istream &stream_;
};
}

std::unique_ptr<utymap::entities::Element> ElementStream::read(std::istream &stream, std::uint64_t id) {
  auto reader = ElementReader(stream);
  auto element = reader.read();
  element->id = id;
  return element;
}

void ElementStream::write(std::ostream &stream, const utymap::entities::Element &element) {
  auto writer = ElementWriter(stream);
  element.accept(writer);
}
