#include "index/ElementStream.hpp"
#include "index/PersistentElementStore.hpp"

#include <fstream>
#include <mutex>

using namespace utymap;
using namespace utymap::index;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::utils;

namespace {
const std::string IndexFileExtension = ".idf";
const std::string DataFileExtension = ".dat";
}

// TODO improve thread safety!
class PersistentElementStore::PersistentElementStoreImpl final {
  struct QuadKeyData {
    std::unique_ptr<std::fstream> dataFile;
    std::unique_ptr<std::fstream> indexFile;

    QuadKeyData(const std::string &dataPath, const std::string &indexPath) :
        dataFile(utymap::utils::make_unique<std::fstream>()),
        indexFile(utymap::utils::make_unique<std::fstream>()) {
      using std::ios;
      dataFile->open(dataPath, ios::in | ios::out | ios::binary | ios::app | ios::ate);
      indexFile->open(indexPath, ios::in | ios::out | ios::binary | ios::app | ios::ate);
    }

    QuadKeyData(const QuadKeyData &) = delete;
    QuadKeyData &operator=(const QuadKeyData &) = delete;

    QuadKeyData(QuadKeyData &&other) :
        dataFile(std::move(other.dataFile)),
        indexFile(std::move(other.indexFile)) {
    }

    ~QuadKeyData() {
      if (dataFile!=nullptr && dataFile->good()) dataFile->close();
      if (indexFile!=nullptr && indexFile->good()) indexFile->close();
    }
  };

 public:
  explicit PersistentElementStoreImpl(const std::string &dataPath) :
      dataPath_(dataPath) {
  }

  void store(const Element &element, const QuadKey &quadKey) {
    auto quadKeyData = createQuadKeyData(quadKey);
    auto offset = static_cast<std::uint32_t>(quadKeyData.dataFile->tellg());

    // write element index
    quadKeyData.indexFile->seekg(0, std::ios::end);
    quadKeyData.indexFile->write(reinterpret_cast<const char *>(&element.id), sizeof(element.id));
    quadKeyData.indexFile->write(reinterpret_cast<const char *>(&offset), sizeof(offset));

    // write element data
    quadKeyData.dataFile->seekg(0, std::ios::end);
    ElementStream::write(*quadKeyData.dataFile, element);
  }

  void search(const QuadKey &quadKey, ElementVisitor &visitor, const utymap::CancellationToken &cancelToken) {
    auto quadKeyData = createQuadKeyData(quadKey);
    auto count = static_cast<std::uint32_t>(quadKeyData.indexFile->tellg()/
        (sizeof(std::uint64_t) + sizeof(std::uint32_t)));

    quadKeyData.indexFile->seekg(0, std::ios::beg);
    for (std::uint32_t i = 0; i < count; ++i) {
      if (cancelToken.isCancelled()) break;

      std::uint64_t id;
      std::uint32_t offset;
      quadKeyData.indexFile->read(reinterpret_cast<char *>(&id), sizeof(id));
      quadKeyData.indexFile->read(reinterpret_cast<char *>(&offset), sizeof(offset));
      quadKeyData.dataFile->seekg(offset, std::ios::beg);

      ElementStream::read(*quadKeyData.dataFile, id)->accept(visitor);
    }
  }

  bool hasData(const QuadKey &quadKey) const {
    std::ifstream file(getFilePath(quadKey, DataFileExtension));
    return file.good();
  }

 private:
  /// Creates quadkey data.
  QuadKeyData createQuadKeyData(const QuadKey &quadKey) const {
    return QuadKeyData(getFilePath(quadKey, DataFileExtension), getFilePath(quadKey, IndexFileExtension));
  }

  /// Gets full file path for given quadkey
  std::string getFilePath(const QuadKey &quadKey, const std::string &extension) const {
    std::stringstream ss;
    ss << dataPath_ << "data/" << quadKey.levelOfDetail << "/" << GeoUtils::quadKeyToString(quadKey) << extension;
    return ss.str();
  }

  const std::string dataPath_;
};

PersistentElementStore::PersistentElementStore(const std::string &dataPath, const StringTable &stringTable) :
    ElementStore(stringTable), pimpl_(utymap::utils::make_unique<PersistentElementStoreImpl>(dataPath)) {
}

PersistentElementStore::~PersistentElementStore() {
}

void PersistentElementStore::storeImpl(const Element &element, const QuadKey &quadKey) {
  pimpl_->store(element, quadKey);
}

void PersistentElementStore::search(const QuadKey &quadKey,
                                    ElementVisitor &visitor,
                                    const utymap::CancellationToken &cancelToken) {
  pimpl_->search(quadKey, visitor, cancelToken);
}

bool PersistentElementStore::hasData(const QuadKey &quadKey) const {
  return pimpl_->hasData(quadKey);
}
