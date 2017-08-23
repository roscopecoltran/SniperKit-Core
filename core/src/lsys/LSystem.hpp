#ifndef LSYS_LSYSTEM_HPP_DEFINED
#define LSYS_LSYSTEM_HPP_DEFINED

#include "lsys/Rules.hpp"

#include <map>
#include <memory>
#include <vector>

namespace utymap {
namespace lsys {

/// Specifies lsystem created from lsys grammar.
struct LSystem final {
  /// Defines rule type;
  typedef std::shared_ptr<Rule> RuleType;
  /// Defines type for list of rules.
  typedef std::vector<RuleType> Rules;
  /// Defines production type with its probability.
  typedef std::vector<std::pair<double, Rules>> Productions;

  /// Amount of generations.
  int generations = 1;

  /// Default turn angle value.
  double angle = 1;

  /// Default scale factor.
  double scale = 1;

  /// Specifies axiom.
  Rules axiom;

  /// Specifies a map where key is word from V* alphabet and value is set of productions.
  std::map<RuleType, Productions, RuleComparator> productions;
};

}
}

#endif // LSYS_LSYSTEM_HPP_DEFINED
