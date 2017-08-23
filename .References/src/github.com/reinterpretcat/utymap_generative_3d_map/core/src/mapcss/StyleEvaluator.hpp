#ifndef MAPCSS_STYLEEVALUATOR_HPP_INCLUDED
#define MAPCSS_STYLEEVALUATOR_HPP_INCLUDED

#include "entities/Element.hpp"
#include "index/StringTable.hpp"
#include "utils/CoreUtils.hpp"
#include "utils/ElementUtils.hpp"

#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/apply_visitor.hpp>

#include <cstdint>
#include <string>
#include <list>
#include <memory>
#include <vector>

namespace utymap {
namespace mapcss {

/// Represents style declaration which support evaluation.
struct StyleEvaluator final {
  /// NOTE has to put these declarations here due to evaluate function implementation
  struct Nil {};
  struct Tag {
    std::string key;
  };

  struct Signed;
  struct Tree;
  typedef boost::variant<Nil, double, std::string, Tag, boost::recursive_wrapper<Signed>> Operand;

  struct Signed {
    char sign;
    Operand operand;
  };

  struct Operation {
    char operator_;
    Operand operand;
  };

  struct Tree {
    Operand first;
    std::list<Operation> rest;
  };

  StyleEvaluator() = delete;

  /// Parses expression into AST.
  static std::unique_ptr<Tree> parse(const std::string &expression);

  /// Evaluates expression using tags.
  template<typename T>
  static T evaluate(const Tree &tree,
                    const std::vector<utymap::entities::Tag> &tags,
                    const utymap::index::StringTable &stringTable) {
    typedef typename std::conditional<std::is_same<T, std::string>::value, StringEvaluator, DoubleEvaluator>::type
        EvaluatorType;
    return EvaluatorType(tags, stringTable).visit(tree);
  }

 private:

  /// Specifies default AST evaluator behaviour.
  template<typename T>
  struct Evaluator {
    typedef T result_type;

    Evaluator(const std::vector<utymap::entities::Tag> &tags,
              const utymap::index::StringTable &stringTable) :
        tags_(tags), stringTable_(stringTable) {}

  protected:
    const std::vector<utymap::entities::Tag> &tags_;
    const utymap::index::StringTable &stringTable_;
  };

  /// Evaluates double from AST.
  struct DoubleEvaluator : public Evaluator<double> {
    DoubleEvaluator(const std::vector<utymap::entities::Tag> &tags,
                    const utymap::index::StringTable &stringTable) :
        Evaluator(tags, stringTable) {}

    double operator()(const Nil&) const {
      return 0;
    }
 
    double operator()(const double &value) const {
      return value;
    }

    double operator()(const std::string &value) const {
      return utymap::utils::parseDouble(value);
    }

    double operator()(const Tag &tag) const {
      auto keyId = stringTable_.getId(tag.key);
      return utymap::utils::parseDouble(utymap::utils::getTagValue(keyId, tags_, stringTable_));
    }

    double operator()(const Signed &s) const {
      double rhs = boost::apply_visitor(*this, s.operand);
      switch (s.sign) {
        case '-': return -rhs;
        case '+': return +rhs;
        default: return 0;
      }
    }

    double visit(const Tree &tree) const {
      double state = boost::apply_visitor(*this, tree.first);
      for (const Operation &op : tree.rest)
        apply(op, state);
      return state;
    }

  private:
    void apply(const Operation &o, double& state) const {
      double rhs = boost::apply_visitor(*this, o.operand);
      switch (o.operator_) {
        case '+': state += rhs; break;
        case '-': state -= rhs; break;
        case '*': state *= rhs; break;
        case '/': state /= rhs; break;
        default: return;
      }
    }
  };

  /// Evaluates string from AST.
  struct StringEvaluator : public Evaluator<std::string> {
    StringEvaluator(const std::vector<utymap::entities::Tag> &tags,
                    const utymap::index::StringTable &stringTable)
        : Evaluator(tags, stringTable) {}

    std::string operator()(const Nil&) const {
      return "";
    }
   
    std::string operator()(const double& value) const {
      return utymap::utils::toString(value);
    }

    std::string operator()(const std::string &value) const {
      return value;
    }

    std::string operator()(const Tag &tag) const {
      return utymap::utils::getTagValue(stringTable_.getId(tag.key), tags_, stringTable_);
    }

    std::string operator()(const Signed &s) const {
      throw std::domain_error("StringEvaluator: sign operation is not supported.");
    }

    std::string visit(const Tree &tree) const {
      auto state = boost::apply_visitor(*this, tree.first);
      for (const Operation &op : tree.rest)
        apply(op, state);
      return state;
    }

  private:
    void apply(const Operation &o, std::string &state) const {
      std::string rhs = boost::apply_visitor(*this, o.operand);
      if (o.operator_ != '+')
        throw std::domain_error(std::string("StringEvaluator: unsupported merge operation: ") + o.operator_);

      state += rhs;
    }
  };

  std::string value_;
  std::unique_ptr<Tree> tree_;
};

}
}
#endif  // MAPCSS_STYLEEVALUATOR_HPP_INCLUDED
