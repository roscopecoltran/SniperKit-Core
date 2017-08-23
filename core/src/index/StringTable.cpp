#include "hashing/MurmurHash3.h"
#include "index/StringTable.hpp"
#include "utils/CoreUtils.hpp"

#include <fstream>
#include <mutex>
#include <unordered_map>

using std::ios;
using namespace utymap::index;

/// Naive implementation of string table: reads all the time string from file; acquires lock
/// TODO optimize it to avoid locks and expensive file reads.
class StringTable::StringTableImpl {
  typedef std::vector<std::uint32_t> IdList;
  typedef std::unordered_map<std::uint32_t, IdList> HashIdMap;

 public:
  StringTableImpl(const std::string &indexPath, const std::string &dataPath, std::uint32_t seed) :
      indexFile_(indexPath, ios::in | ios::out | ios::binary | ios::ate | ios::app),
      dataFile_(dataPath, ios::in | ios::out | ios::binary | ios::app),
      seed_(seed),
      nextId_(0),
      map_(),
      offsets_() {
    nextId_ = static_cast<std::uint32_t>(indexFile_.tellg()/(sizeof(std::uint32_t)*2));
    if (nextId_ > 0) {
      std::uint32_t count = nextId_;
      indexFile_.seekg(0, ios::beg);

      // NOTE reserve some extra size for possible insertions
      std::size_t capacity = count + 2048;
      offsets_.reserve(capacity);
      map_.reserve(capacity);

      for (std::uint32_t i = 0; i < count; ++i) {
        std::uint32_t hash, offset;
        indexFile_.read(reinterpret_cast<char *>(&hash), sizeof(hash));
        indexFile_.read(reinterpret_cast<char *>(&offset), sizeof(offset));
        offsets_.push_back(offset);
        map_[hash].push_back(i);
      }
    }
  }

  std::uint32_t getId(const std::string &str) {
    std::uint32_t hash;
    MurmurHash3_x86_32(str.c_str(), static_cast<int>(str.size()), seed_, &hash);

    // TODO avoid lock there
    std::lock_guard<std::mutex> lock(lock_);
    HashIdMap::iterator hashLookupResult = map_.find(hash);
    if (hashLookupResult!=map_.end()) {
      std::string data;
      for (std::uint32_t id : hashLookupResult->second) {
        data.clear();
        readString(id, data);
        if (str==data)
          return id;
      }
    }

    writeString(hash, str);
    return nextId_++;
  }

  std::string getString(std::uint32_t id) {
    std::string str;
    // TODO avoid lock there
    std::lock_guard<std::mutex> lock(lock_);
    readString(id, str);
    return str;
  }

 private:

  /// Reads string by id.
  void readString(std::uint32_t id, std::string &data) {
    if (id < offsets_.size()) {
      std::uint32_t offset = offsets_[id];
      dataFile_.seekg(offset, ios::beg);
      std::getline(dataFile_, data, '\0');
    }
  }

  /// Writes string to index.
  void writeString(std::uint32_t hash, const std::string &data) {
    // get offset as file size
    dataFile_.seekg(0, ios::end);
    std::uint32_t offset = static_cast<std::uint32_t>(dataFile_.tellg());

    // write string
    dataFile_.seekp(0, ios::end);
    dataFile_ << data.c_str() << '\0';

    // write index entry
    indexFile_.seekp(0, ios::end);
    indexFile_.write(reinterpret_cast<char *>(&hash), sizeof(hash));
    indexFile_.write(reinterpret_cast<char *>(&offset), sizeof(offset));

    map_[hash].push_back(nextId_);
    offsets_.push_back(offset);
  }

  std::fstream indexFile_;
  std::fstream dataFile_;
  std::uint32_t seed_;
  std::uint32_t nextId_;

  /// TODO think about better data structure alternatives
  HashIdMap map_;
  std::vector<std::uint32_t> offsets_;

  std::mutex lock_;
};

StringTable::StringTable(const std::string &path) :
    pimpl_(utymap::utils::make_unique<StringTableImpl>(path + "string.idx", path + "string.dat", 0)) {
}

StringTable::~StringTable() {}

std::uint32_t StringTable::getId(const std::string &str) const {
  return pimpl_->getId(str);
}

std::string StringTable::getString(std::uint32_t id) const {
  return pimpl_->getString(id);
}
