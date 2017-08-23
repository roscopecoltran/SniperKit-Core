#include "lsys/LSystem.hpp"

#include <random>

using namespace utymap::lsys;

namespace {

/// Provides the way to select productions based on their probability.
struct RuleSelector {
  RuleSelector() : rd(), gen(rd()) {
  }

  const LSystem::Rules &operator()(const LSystem::Productions &productions) {
    std::vector<double> weights;
    for (const auto &p : productions)
      weights.push_back(p.first);

    // NOTE replacement code in VS2013 for: std::discrete_distribution<> dist(first, last);
    auto first = weights.cbegin();
    auto count = std::distance(first, weights.cend());
    std::discrete_distribution<> dist(count, -0.5, -0.5 + count, [&first](size_t i) {
      return *std::next(first, i);
    });

    return productions.at(dist(gen)).second;
  }
 private:
  std::random_device rd;
  std::mt19937 gen;
};

}

void Turtle::run(const LSystem &lsystem) {
  // generate rules from axiom based on amount of generations and productions.
  RuleSelector ruleSelector;
  auto rules = lsystem.axiom;
  for (int i = 0; i < lsystem.generations; ++i) {
    LSystem::Rules current;
    for (const auto &rule : rules) {
      const auto value = lsystem.productions.find(rule);
      if (value==lsystem.productions.end()) {
        current.push_back(rule);
        continue;
      }
      const auto &productions = ruleSelector(value->second);
      current.insert(current.end(), productions.begin(), productions.end());
    }
    rules = std::move(current);
  }

  // visit all rules applying turtle's actions.
  for (const auto &rule : rules)
    rule->apply(*this);
}
