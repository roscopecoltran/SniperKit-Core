#ifndef INDEX_INMEMORYELEMENTSTORE_HPP_DEFINED
#define INDEX_INMEMORYELEMENTSTORE_HPP_DEFINED

#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "index/ElementStore.hpp"

#include <memory>

namespace utymap {
namespace index {

/// Provides API to store elements in memory.
class InMemoryElementStore final : public ElementStore {
 public:
  explicit InMemoryElementStore(const utymap::index::StringTable &stringTable);

  virtual ~InMemoryElementStore();

  void search(const utymap::QuadKey &quadKey,
              utymap::entities::ElementVisitor &visitor,
              const utymap::CancellationToken &cancelToken) override;

  bool hasData(const utymap::QuadKey &quadKey) const override;

 protected:
  void storeImpl(const utymap::entities::Element &element, const utymap::QuadKey &quadKey) override;

 private:
  class InMemoryElementStoreImpl;
  std::unique_ptr<InMemoryElementStoreImpl> pimpl_;
};

}
}

#endif // INDEX_INMEMORYELEMENTSTORE_HPP_DEFINED
