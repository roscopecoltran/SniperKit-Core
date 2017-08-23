#ifndef INDEX_STRINGTABLE_HPP_INCLUDED
#define INDEX_STRINGTABLE_HPP_INCLUDED

#include <cstdint>
#include <string>
#include <memory>

namespace utymap {
namespace index {

/// Provides the way to maintain strings.
/// Index file consists of id-offset pairs where id - string id,
/// offset - first character of the string inside data file.
/// data file contains list of null terminated strings.
class StringTable final {
 public:

  /// Creates instance of StringTable using file path provided.
  explicit StringTable(const std::string &path);

  StringTable(const StringTable &) = delete;
  StringTable &operator=(StringTable const &) = delete;

  ~StringTable();

  /// Gets id of given string.
  std::uint32_t getId(const std::string &str) const;

  /// Gets original string by id.
  std::string getString(std::uint32_t id) const;

 private:
  class StringTableImpl;
  std::unique_ptr<StringTableImpl> pimpl_;
};

}
}
#endif  // INDEX_STRINGTABLE_HPP_INCLUDED
