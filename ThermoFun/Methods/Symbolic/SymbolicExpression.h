// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_SYMBOLIC_SYMBOLICEXPRESSION_H
#define THERMOFUN_METHODS_SYMBOLIC_SYMBOLICEXPRESSION_H

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <ThermoFun/Common/Real.hpp>

namespace ThermoFun {

/// An error of a symbolic expression with the code of the diagnostic (V2-SYM-001, 002, 006 or V2-MTH-008)
struct SymbolicError : std::runtime_error
{
    std::string code;
    SymbolicError(std::string code, const std::string& message) : std::runtime_error(message), code(std::move(code)) {}
};

/// The size limits of an expression
struct SymbolicLimits
{
    size_t max_characters = 2000;
    int max_depth = 50;
    int max_operations = 1000;
};

namespace symbolic {

/// One node of the evaluation tree. SymEngine does not appear in this header.
struct Node
{
    enum class Op { Constant, Variable, Add, Mul, Pow, Exp, Log, Abs };
    Op op = Op::Constant;
    double value = 0.0;       ///< for Constant
    int variable = -1;        ///< for Variable: the position in the list of values
    std::vector<std::shared_ptr<const Node>> children;
};

using NodePtr = std::shared_ptr<const Node>;

/// The power of numbers or of autodiff numbers. An exponent without a derivative is a constant.
template <class S>
inline auto power(const S& x, const S& y) -> S
{
    using std::pow;
    using ThermoFun::pow;
    return pow(x, y);
}

template <class S>
auto evaluate(const Node& node, const std::vector<S>& values) -> S
{
    using std::abs;
    using std::exp;
    using std::log;
    switch (node.op)
    {
    case Node::Op::Constant: return S(node.value);
    case Node::Op::Variable: return values[node.variable];
    case Node::Op::Add:
    {
        S sum = evaluate<S>(*node.children[0], values);
        for (size_t i = 1; i < node.children.size(); ++i)
            sum = sum + evaluate<S>(*node.children[i], values);
        return sum;
    }
    case Node::Op::Mul:
    {
        S product = evaluate<S>(*node.children[0], values);
        for (size_t i = 1; i < node.children.size(); ++i)
            product = product * evaluate<S>(*node.children[i], values);
        return product;
    }
    case Node::Op::Pow: return power<S>(evaluate<S>(*node.children[0], values), evaluate<S>(*node.children[1], values));
    case Node::Op::Exp: return exp(evaluate<S>(*node.children[0], values));
    case Node::Op::Log: return log(evaluate<S>(*node.children[0], values));
    case Node::Op::Abs: return abs(evaluate<S>(*node.children[0], values));
    }
    return S(0.0);
}

}

/// A closed-form expression of T, P and named values. It is parsed once with SymEngine, differentiated symbolically and
/// evaluated by a small tree walker on numbers or on autodiff numbers.
class SymbolicExpression
{
public:
    /// True if the library was built with SymEngine (the option TFUN_USE_SYMENGINE)
    static auto available() -> bool;

    /// Parses `text`. Only the `variables` and the functions + - * / ** exp log sqrt pow abs are allowed.
    /// Throws SymbolicError V2-SYM-001 (unknown name), V2-SYM-002 (parse error), V2-SYM-006 (limits)
    /// or V2-MTH-008 (built without SymEngine).
    static auto parse(const std::string& text, const std::vector<std::string>& variables, const SymbolicLimits& limits = {})
        -> SymbolicExpression;

    /// The derivative with respect to one of the variables
    auto diff(const std::string& variable) const -> SymbolicExpression;

    /// The value for the `values` given in the order of the variables
    template <class S>
    auto evaluate(const std::vector<S>& values) const -> S
    {
        return symbolic::evaluate<S>(*root_, values);
    }

    auto variables() const -> const std::vector<std::string>& { return variables_; }
    auto text() const -> const std::string& { return text_; }

private:
    friend struct SymbolicAccess;
    symbolic::NodePtr root_;
    std::vector<std::string> variables_;
    std::string text_;
    SymbolicLimits limits_;
    std::shared_ptr<void> engine_;   ///< the SymEngine expression, owned here and unknown to the header
};

}

#endif
