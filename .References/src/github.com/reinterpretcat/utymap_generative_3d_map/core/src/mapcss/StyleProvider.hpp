#ifndef INDEX_STYLEPROVIDER_HPP_DEFINED
#define INDEX_STYLEPROVIDER_HPP_DEFINED

#include "index/StringTable.hpp"
#include "entities/Element.hpp"
#include "mapcss/ColorGradient.hpp"
#include "mapcss/StyleSheet.hpp"
#include "mapcss/Style.hpp"
#include "lsys/LSystem.hpp"

#include <string>
#include <memory>

namespace utymap {
namespace mapcss {

/// This class responsible for providing element styles.
class StyleProvider final {
 public:

  StyleProvider(const StyleSheet &,
                utymap::index::StringTable &);

  ~StyleProvider();
  StyleProvider(StyleProvider &&);

  StyleProvider(const StyleProvider &) = delete;
  StyleProvider &operator=(const StyleProvider &) = delete;
  StyleProvider &operator=(StyleProvider &&) = delete;

  /// Returns an unique tag associated with the used styles.
  const std::string &getTag() const;

  /// Checks whether style is defined for the element.
  bool hasStyle(const utymap::entities::Element &, int levelOfDetails) const;

  /// Returns style for given element at given level of details.
  Style forElement(const utymap::entities::Element &, int levelOfDetails) const;

  /// Returns style for canvas at given level of details.
  Style forCanvas(int levelOfDetails) const;

  /// Returns color gradient for given key.
  const ColorGradient &getGradient(const std::string &key) const;

  /// Returns texture group from given atlas using key.
  const TextureGroup &getTexture(std::uint16_t index, const std::string &key) const;

  /// Returns lsystem with given id.
  const utymap::lsys::LSystem &getLsystem(const std::string &key) const;

 private:
  class StyleProviderImpl;
  std::unique_ptr<StyleProviderImpl> pimpl_;
};

}
}

#endif // INDEX_STYLEPROVIDER_HPP_DEFINED
