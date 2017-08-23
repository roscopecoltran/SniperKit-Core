#include "hashing/MD5.h"
#include "entities/ElementVisitor.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "mapcss/Style.hpp"
#include "mapcss/StyleProvider.hpp"
#include "utils/GradientUtils.hpp"

#include <mutex>

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

namespace {

const std::uint16_t DefaultTextureIndex = std::numeric_limits<std::uint16_t>::max();

/// Contains operation types supported by mapcss parser.
enum class OpType { Exists, Equals, NotEquals, Less, Greater };

struct ConditionType final {
  uint32_t key;
  uint32_t value;
  OpType type;
};

struct ConditionFilter final {
  std::vector<ConditionType> conditions;
  std::vector<std::shared_ptr<const StyleDeclaration>> declarations;
};

typedef std::vector<std::shared_ptr<const StyleDeclaration>> StyleDeclarations;
/// Key: level of details, value: filters for specific element type.
typedef std::unordered_map<int, std::vector<ConditionFilter>> ConditionFilterMap;
typedef std::unordered_map<std::uint64_t, StyleDeclarations> IdentifierFilter;
typedef std::unordered_map<int, IdentifierFilter> IdentifierFilterMap;

struct FilterCollection final {
  ConditionFilterMap nodes;
  ConditionFilterMap ways;
  ConditionFilterMap areas;
  ConditionFilterMap relations;
  ConditionFilterMap canvases;
  IdentifierFilterMap elements;
};

/// Alters tag with value specific for passed style declarations.
void addTo(std::string &tag, const StyleDeclarations &styles) {
  for (const auto &style :styles) {
    tag.append(utymap::utils::toString(style->key()));
    tag.append(utymap::utils::toString(style->value()));
  }
}

void addTo(std::string &tag, const OpType &type) {
  switch (type) {
    case OpType::Exists: tag.append("~");
      break;
    case OpType::Equals: tag.append("==");
      break;
    case OpType::NotEquals: tag.append("!=");
      break;
    case OpType::Less: tag.append("<");
      break;
    case OpType::Greater: tag.append(">");
      break;
  }
}

/// Alters tag with value specific for passed filter map.
void addTo(std::string &tag, const ConditionFilterMap &filterMap) {
  for (const auto &pair : filterMap) {
    tag.append(utymap::utils::toString(pair.first));
    for (const auto &filter : pair.second) {
      for (const auto &cond : filter.conditions) {
        tag.append(utymap::utils::toString(cond.key));
        addTo(tag, cond.type);
        tag.append(utymap::utils::toString(cond.value));
      }
      addTo(tag, filter.declarations);
    }
  }
}

/// Alters tag with value specific for passed filter map.
void addTo(std::string &tag, const IdentifierFilterMap &filterMap) {
  for (const auto &pair : filterMap) {
    tag.append(utymap::utils::toString(pair.first));
    for (const auto &id : pair.second) {
      tag.append(utymap::utils::toString(id.first));
      addTo(tag, id.second);
    }
  }
}

/// Gets hash for given filter collection.
std::string getHashTag(const FilterCollection &filterCollection) {
  std::string tag;
  tag.reserve(64000);

  addTo(tag, filterCollection.nodes);
  addTo(tag, filterCollection.ways);
  addTo(tag, filterCollection.areas);
  addTo(tag, filterCollection.relations);
  addTo(tag, filterCollection.canvases);
  addTo(tag, filterCollection.elements);

  return MD5(tag).hexdigest();
}

class StyleBuilder final : public ElementVisitor {
  typedef std::vector<Tag>::const_iterator TagIterator;
 public:

  StyleBuilder(std::vector<Tag> tags, StringTable &stringTable,
               const FilterCollection &filters, int levelOfDetail, bool onlyCheck = false) :
      style(tags, stringTable),
      filters_(filters),
      levelOfDetail_(levelOfDetail),
      onlyCheck_(onlyCheck),
      canBuild_(false),
      stringTable_(stringTable) {
  }

  void visitNode(const Node &node) override { checkOrBuild(node, filters_.nodes); }

  void visitWay(const Way &way) override { checkOrBuild(way, filters_.ways); }

  void visitArea(const Area &area) override { checkOrBuild(area, filters_.areas); }

  void visitRelation(const Relation &relation) override { checkOrBuild(relation, filters_.relations); }

  bool canBuild() const { return canBuild_; }

  Style style;

 private:

  void checkOrBuild(const Element &element, const ConditionFilterMap &filters) {
    if (!buildFromIdentifier(element))
      buildFromCondition(element.tags, filters);
  }

  /// Checks tag's value assuming that the key is already checked.
  bool matchTag(const Tag &tag, const ConditionType &condition) {
    switch (condition.type) {
      case OpType::Exists:return true;
      case OpType::Equals:return tag.value==condition.value;
      case OpType::NotEquals:return tag.value!=condition.value;
      case OpType::Less:return compareDoubles(tag.value, condition.value, std::less<double>());
      case OpType::Greater:return compareDoubles(tag.value, condition.value, std::greater<double>());
      default:return false;
    }
  }

  /// Compares two raw string values using double conversion.
  template<typename Func>
  bool compareDoubles(std::uint32_t left, std::uint32_t right, Func binaryOp) {
    double leftValue = utymap::utils::parseDouble(stringTable_.getString(left));
    double rightValue = utymap::utils::parseDouble(stringTable_.getString(right));

    return binaryOp(leftValue, rightValue);
  }

  /// Tries to find tag which satisfy condition using binary search.
  bool matchTags(TagIterator begin, TagIterator end, const ConditionType &condition) {
    while (begin < end) {
      const TagIterator middle = begin + (std::distance(begin, end)/2);
      if (middle->key==condition.key)
        return matchTag(*middle, condition);
      else if (middle->key > condition.key)
        end = middle;
      else
        begin = middle + 1;
    }
    return false;
  }

  /// Builds style object from regular mapcss rule encapsulated by condition filter.
  void buildFromCondition(const std::vector<Tag> &tags, const ConditionFilterMap &filters) {
    ConditionFilterMap::const_iterator iter = filters.find(levelOfDetail_);
    if (iter!=filters.end()) {
      for (const ConditionFilter &filter : iter->second) {
        bool isMatched = true;
        for (auto it = filter.conditions.cbegin(); it!=filter.conditions.cend() && isMatched; ++it) {
          isMatched &= matchTags(tags.cbegin(), tags.cend(), *it);
        }
        // merge declarations to style
        if (isMatched) {
          canBuild_ = true;
          if (onlyCheck_) return;

          for (const auto &d : filter.declarations) {
            style.put(*d);
          }
        }
      }
    }
  }

  /// Builds style object from element id rule encapsulated by identifier filter.
  bool buildFromIdentifier(const Element &element) {
    auto filterMap = filters_.elements.find(levelOfDetail_);
    if (filterMap!=filters_.elements.end()) {
      auto elementStyle = filterMap->second.find(element.id);
      if (elementStyle!=filterMap->second.end()) {
        canBuild_ = true;
        if (!onlyCheck_) {
          for (const auto &d : elementStyle->second)
            style.put(*d);
        }
        return true;
      }
    }
    return false;
  }

  const FilterCollection &filters_;
  int levelOfDetail_;
  bool onlyCheck_;
  bool canBuild_;
  StringTable &stringTable_;
};

}

/// Converts mapcss stylesheet to index optimized representation to speed search query up.
class StyleProvider::StyleProviderImpl {
 public:

  FilterCollection filters;
  StringTable &stringTable;

  StyleProviderImpl(const StyleSheet &stylesheet, StringTable &stringTable) :
      filters(),
      stringTable(stringTable),
      gradients(),
      textures() {
    filters.nodes.reserve(24);
    filters.ways.reserve(24);
    filters.areas.reserve(24);
    filters.relations.reserve(24);
    filters.canvases.reserve(24);
    filters.elements.reserve(24);

    for (const Rule &rule : stylesheet.rules) {
      for (const Selector &selector : rule.selectors) {
        for (const std::string &name : selector.names) {
          ConditionFilterMap *filtersPtr = nullptr;
          if (name=="node") filtersPtr = &filters.nodes;
          else if (name=="way") filtersPtr = &filters.ways;
          else if (name=="area") filtersPtr = &filters.areas;
          else if (name=="relation") filtersPtr = &filters.relations;
          else if (name=="canvas") filtersPtr = &filters.canvases;
          else if (name=="element") {
            addIdentifierRule(selector, rule.declarations);
            continue;
          } else
            throw std::domain_error("Unexpected selector name:" + name);

          addConditionRule(filtersPtr, rule, selector);
        }
      }
    }

    textures.emplace(DefaultTextureIndex, utymap::utils::make_unique<const TextureAtlas>());
    for (const auto &texture: stylesheet.textures) {
      textures.emplace(texture.index(), utymap::utils::make_unique<const TextureAtlas>(texture));
    }

    for (const auto &lsystem : stylesheet.lsystems) {
      lsystems.emplace(lsystem.first, utymap::utils::make_unique<const utymap::lsys::LSystem>(lsystem.second));
    }

    hashTag_ = getHashTag(filters);
  }

  const std::string &getTag() const {
    return hashTag_;
  }

  const ColorGradient &getGradient(const std::string &key) {
    auto gradientPair = gradients.find(key);
    if (gradientPair==gradients.end()) {
      auto gradient = utymap::utils::GradientUtils::parseGradient(key);
      if (gradient->empty())
        throw MapCssException("Invalid gradient: " + key);
      std::lock_guard<std::mutex> lock(lock_);
      gradients.emplace(key, std::move(gradient));
      gradientPair = gradients.find(key);
    }
    return *gradientPair->second;
  }

  const TextureGroup &getTexture(std::uint16_t index, const std::string &key) const {
    auto texturePair = textures.find(index);
    if (texturePair==textures.end())
      texturePair = textures.find(DefaultTextureIndex);

    return texturePair->second->get(key);
  }

  const utymap::lsys::LSystem &getLsystem(const std::string &key) const {
    auto lsystemPair = lsystems.find(key);
    if (lsystemPair==lsystems.end())
      throw MapCssException("Invalid lsystem: " + key);

    return *lsystemPair->second;
  }

 private:

  /// Adds rule for element with specific id.
  void addIdentifierRule(const Selector &selector, const std::vector<Declaration> &declarations) {
    auto filter = IdentifierFilter();
    addDeclarations(declarations, [&](std::shared_ptr<const StyleDeclaration> declaration) {
      auto id = utymap::utils::lexicalCast<std::uint64_t>(selector.conditions[0].value);
      filter[id].push_back(declaration);
    });

    for (int i = selector.zoom.start; i <= selector.zoom.end; ++i) {
      for (const auto &filterPair : filter)
        filters.elements[i].emplace(filterPair);
    }
  }

  /// Adds rule for element.
  void addConditionRule(ConditionFilterMap *filtersPtr, const Rule &rule, const Selector &selector) {
    ConditionFilter filter;
    addConditions(filter, selector.conditions);
    addDeclarations(rule.declarations, [&](std::shared_ptr<const StyleDeclaration> declaration) {
      filter.declarations.push_back(declaration);
    });
    addToFilterMap(filtersPtr, filter, selector);
  }

  void addConditions(ConditionFilter &filter, const std::vector<Condition> &conditions) {
    filter.conditions.reserve(conditions.size());
    for (const Condition &condition : conditions) {
      ConditionType c;
      if (condition.operation.empty()) c.type = OpType::Exists;
      else if (condition.operation=="=") c.type = OpType::Equals;
      else if (condition.operation=="!=") c.type = OpType::NotEquals;
      else if (condition.operation=="<") c.type = OpType::Less;
      else if (condition.operation==">") c.type = OpType::Greater;
      else
        throw std::domain_error("Unexpected condition operation:" + condition.operation);

      c.key = stringTable.getId(condition.key);
      c.value = stringTable.getId(condition.value);
      filter.conditions.push_back(c);
    }
  }

  template<typename T>
  void addDeclarations(const std::vector<Declaration> &declarations, const T &filter) {
    for (const auto &declaration : declarations) {
      if (utymap::utils::GradientUtils::isGradient(declaration.value))
        addGradient(declaration.value);
      filter(std::make_shared<const StyleDeclaration>(stringTable.getId(declaration.key), declaration.value));
    }
  }

  void addToFilterMap(ConditionFilterMap *filtersPtr, ConditionFilter &filter, const Selector &selector) {
    std::sort(filter.conditions.begin(), filter.conditions.end(),
              [](const ConditionType &c1, const ConditionType &c2) { return c1.key > c2.key; });
    for (int i = selector.zoom.start; i <= selector.zoom.end; ++i) {
      (*filtersPtr)[i].push_back(filter);
    }
  }

  void addGradient(const std::string &key) {
    if (gradients.find(key)==gradients.end()) {
      auto gradient = utymap::utils::GradientUtils::parseGradient(key);
      if (!gradient->empty())
        gradients.emplace(key, std::move(gradient));
    }
  }

  std::mutex lock_;
  std::string hashTag_;

  std::unordered_map<std::string, std::unique_ptr<const ColorGradient>> gradients;
  std::unordered_map<std::uint16_t, std::unique_ptr<const TextureAtlas>> textures;
  std::unordered_map<std::string, std::unique_ptr<const utymap::lsys::LSystem>> lsystems;
};

StyleProvider::StyleProvider(const StyleSheet &stylesheet, StringTable &stringTable) :
    pimpl_(utymap::utils::make_unique<StyleProviderImpl>(stylesheet, stringTable)) {
}

StyleProvider::~StyleProvider() {
}

StyleProvider::StyleProvider(StyleProvider &&other) : pimpl_(std::move(other.pimpl_)) {
}

const std::string &StyleProvider::getTag() const {
  return pimpl_->getTag();
}

bool StyleProvider::hasStyle(const utymap::entities::Element &element, int levelOfDetails) const {
  StyleBuilder builder(element.tags, pimpl_->stringTable, pimpl_->filters, levelOfDetails, true);
  element.accept(builder);
  return builder.canBuild();
}

Style StyleProvider::forElement(const Element &element, int levelOfDetails) const {
  StyleBuilder builder(element.tags, pimpl_->stringTable, pimpl_->filters, levelOfDetails);
  element.accept(builder);
  return std::move(builder.style);
}

Style StyleProvider::forCanvas(int levelOfDetails) const {
  Style style({}, pimpl_->stringTable);
  for (const auto &filter : pimpl_->filters.canvases[levelOfDetails]) {
    for (const auto &declaration : filter.declarations) {
      style.put(*declaration);
    }
  }
  return std::move(style);
}

const ColorGradient &StyleProvider::getGradient(const std::string &key) const {
  return pimpl_->getGradient(key);
}

const TextureGroup &StyleProvider::getTexture(std::uint16_t index, const std::string &key) const {
  return pimpl_->getTexture(index, key);
}

const utymap::lsys::LSystem &StyleProvider::getLsystem(const std::string &key) const {
  return pimpl_->getLsystem(key);
}
