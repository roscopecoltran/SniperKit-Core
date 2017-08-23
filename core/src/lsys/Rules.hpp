#ifndef LSYS_RULES_HPP_DEFINED
#define LSYS_RULES_HPP_DEFINED

#include "lsys/Turtle.hpp"

#include <memory>
#include <string>
#include <typeindex>

namespace utymap {
namespace lsys {

/// Specifies base class for rule visiting.
class Rule {
 public:
  /// Applies the rule to turtle.
  virtual void apply(Turtle &turtle) const = 0;

  virtual ~Rule() {}
};

class MoveForwardRule final : public Rule {
 public:
  void apply(Turtle &turtle) const override {
    turtle.moveForward();
  }
};

class JumpForwardRule final : public Rule {
 public:
  void apply(Turtle &turtle) const override {
    turtle.jumpForward();
  }
};

class TurnLeftRule final : public Rule { public: void apply(Turtle &turtle) const override { turtle.turnLeft(); }};

class TurnRightRule final : public Rule { public: void apply(Turtle &turtle) const override { turtle.turnRight(); }};

class TurnAroundRule final : public Rule { public: void apply(Turtle &turtle) const override { turtle.turnAround(); }};

class PitchUpRule final : public Rule { public: void apply(Turtle &turtle) const override { turtle.pitchUp(); }};

class PitchDownRule final : public Rule { public: void apply(Turtle &turtle) const override { turtle.pitchDown(); }};

class RollLeftRule final : public Rule { public: void apply(Turtle &turtle) const override { turtle.rollLeft(); }};

class RollRightRule final : public Rule { public: void apply(Turtle &turtle) const override { turtle.rollRight(); }};

class IncrementRule final : public Rule { public: void apply(Turtle &turtle) const override { turtle.increment(); }};

class DecrementRule final : public Rule { public: void apply(Turtle &turtle) const override { turtle.decrement(); }};

class SwitchStyleRule final : public Rule {
 public:
  void apply(Turtle &turtle) const override {
    turtle.switchStyle();
  }
};

class ScaleUpRule final : public Rule { public: void apply(Turtle &turtle) const override { turtle.scaleUp(); }};

class ScaleDownRule final : public Rule { public: void apply(Turtle &turtle) const override { turtle.scaleDown(); }};

class SaveRule final : public Rule { public: void apply(Turtle &turtle) const override { turtle.save(); }};

class RestoreRule final : public Rule { public: void apply(Turtle &turtle) const override { turtle.restore(); }};

/// Defines rule for custom words.
class WordRule final : public Rule {
 public:
  WordRule(const std::string &word) : word(word) {
  }

  void apply(Turtle &turtle) const override { turtle.say(word); }

  const std::string word;
};

/// Defines rule comparator which can distinguish different rules taking care about specific word rule.
struct RuleComparator {
  bool operator()(const std::shared_ptr<Rule> &left, const std::shared_ptr<Rule> &right) const {
    auto leftType = std::type_index(typeid(*left));
    auto rightType = std::type_index(typeid(*right));

    if (leftType==wordRuleTypeIndex_ && rightType==wordRuleTypeIndex_)
      return static_cast<const WordRule &>(*left).word < static_cast<const WordRule &>(*right).word;

    return leftType < rightType;
  }
 private:
  const std::type_index wordRuleTypeIndex_ = std::type_index(typeid(WordRule));
};

}
}

#endif // LSYS_RULES_HPP_DEFINED
