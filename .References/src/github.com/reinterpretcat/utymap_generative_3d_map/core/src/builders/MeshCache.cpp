#include "builders/MeshCache.hpp"
#include "index/ElementStream.hpp"
#include "index/MeshStream.hpp"

#include <fstream>
#include <mutex>

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::math;
using namespace utymap::utils;

namespace {
const char ElementType = 0;
const char MeshType = 1;
}

class MeshCache::MeshCacheImpl {
  using MeshCallback = BuilderContext::MeshCallback;
  using ElementCallback = BuilderContext::ElementCallback;

 public:
  explicit MeshCacheImpl(const std::string &dataPath, const std::string &extension) :
      dataPath_(dataPath),
      extension_('.' + extension) {}

  BuilderContext wrap(const BuilderContext &context) {
    auto filePath = getFilePath(context);

    std::lock_guard<std::mutex> lock(lock_);
    return isCacheHit(context.quadKey, filePath) ? context : wrap(context, filePath);
  }

  bool fetch(const BuilderContext &context) {
    auto filePath = getFilePath(context);

    {
      std::lock_guard<std::mutex> lock(lock_);
      if (!isCacheHit(context.quadKey, filePath))
        return false;
    }

    readCache(filePath, context);

    return true;
  }

  void unwrap(const BuilderContext &context) {
    std::lock_guard<std::mutex> lock(lock_);

    auto entry = cachingQuads_.find(context.quadKey);

    if (entry==cachingQuads_.end()) return;

    if (entry->second->good()) {
      if (context.cancelToken.isCancelled()) {
        // NOTE no guarantee that all data was processed and saved.
        // So it is better to delete the whole file
        entry->second->close();
        std::remove(getFilePath(context).c_str());
      } else {
        entry->second->seekg(0, std::ios::beg);
        *entry->second << static_cast<char>(1);
        entry->second->close();
      }
    }

    cachingQuads_.erase(entry);
  }

 private:

  /// Checks whether the data associated with given context is already cached on disk.
  bool isCacheHit(const QuadKey &quadKey, const std::string &filePath) const {
    // NOTE if quadkey is preset in collection, then caching is in progress.
    // in this case, we let app to behaviour as there is no cache at all
    if (cachingQuads_.find(quadKey) != cachingQuads_.end()) return false;
    // NOTE if file is on disk, it should be processed.
    std::ifstream file(filePath, std::ios::in | std::ios::binary | std::ios::ate);
    file.seekg(0, std::ios::beg);
    return isGood(file);
  }

  /// Gets path to cache file on disk.
  std::string getFilePath(const BuilderContext &context) const {
    std::stringstream ss;
    ss << dataPath_ << "cache/" << context.styleProvider.getTag()
       << "/" << context.quadKey.levelOfDetail << "/"
       << GeoUtils::quadKeyToString(context.quadKey) << extension_;
    return ss.str();
  }

  BuilderContext wrap(const BuilderContext &context, const std::string &filePath) {
    auto file = std::make_shared<std::fstream>();
    file->open(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
    // NOTE marker that processing in progress
    *file << static_cast<char>(0);

    cachingQuads_.insert({context.quadKey, file});

    return BuilderContext(
        context.quadKey,
        context.styleProvider,
        context.stringTable,
        context.eleProvider,
        wrap(*file, context.meshCallback, context.cancelToken),
        wrap(*file, context.elementCallback, context.cancelToken),
        context.cancelToken);
  }

  static MeshCallback wrap(std::ostream &stream, const MeshCallback &callback, const CancellationToken &token) {
    return [&](const Mesh &mesh) {
      if (token.isCancelled()) return;
      stream << MeshType;
      MeshStream::write(stream, mesh);
      callback(mesh);
    };
  }

  static ElementCallback wrap(std::ostream &stream, const ElementCallback &callback, const CancellationToken &token) {
    return [&](const Element &element) {
      if (token.isCancelled()) return;
      stream << ElementType;
      stream.write(reinterpret_cast<const char *>(&element.id), sizeof(element.id));
      ElementStream::write(stream, element);
      callback(element);
    };
  }

  static void readCache(std::string &filePath, const BuilderContext &context) {
    std::fstream file;
    file.open(filePath, std::ios::in | std::ios::binary | std::ios::app | std::ios::ate);
    file.seekg(0, std::ios::beg);
    
    if (!isGood(file)) return;

    while (!context.cancelToken.isCancelled()) {
      char type;
      if (!(file >> type)) break;

      if (type==MeshType)
        context.meshCallback(MeshStream::read(file));
      else if (type==ElementType) {
        std::uint64_t id;
        file.read(reinterpret_cast<char *>(&id), sizeof(id));
        context.elementCallback(*ElementStream::read(file, id));
      } else
        throw std::invalid_argument("Cannot read cache.");
    }
  }

  static bool isGood(std::istream& stream) {
    if (!stream.good()) return false;

    char status;
    stream >> status;
    return status == 1;
  }

  const std::string dataPath_;
  const std::string extension_;
  std::mutex lock_;
  std::map<QuadKey, std::shared_ptr<std::fstream>, QuadKey::Comparator> cachingQuads_;
};

MeshCache::MeshCache(const std::string &directory, const std::string &extension) :
    pimpl_(utymap::utils::make_unique<MeshCacheImpl>(directory, extension)),
    isEnabled_(true) {}

BuilderContext MeshCache::wrap(const BuilderContext &context) const {
  return isEnabled_ ? pimpl_->wrap(context) : context;
}

bool MeshCache::fetch(const BuilderContext &context) const {
  return isEnabled_ && pimpl_->fetch(context);
}

void MeshCache::unwrap(const BuilderContext &context) const {
  pimpl_->unwrap(context);
}

MeshCache::~MeshCache() {}
