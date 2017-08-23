#ifndef CANCELLATIONTOKEN_HPP_DEFINED
#define CANCELLATIONTOKEN_HPP_DEFINED

namespace utymap {

/// Cancellation token passed from outside.
struct CancellationToken final {
 private:
  /// Non-zero value means cancellation.
  int cancelled = 0;

 public:
  /// Helper method to detect cancellation.
  bool isCancelled() const {
    return cancelled!=0;
  }
};

}

#endif // CANCELLATIONTOKEN_HPP_DEFINED
