// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "SymbolicExpression.h"

#include <algorithm>
#include <regex>
#include <set>

#ifdef TFUN_WITH_SYMENGINE
#include <symengine/add.h>
#include <symengine/basic.h>
#include <symengine/constants.h>
#include <symengine/eval_double.h>
#include <symengine/functions.h>
#include <symengine/mul.h>
#include <symengine/parser.h>
#include <symengine/pow.h>
#include <symengine/symbol.h>
#include <symengine/printers.h>
#endif

namespace ThermoFun {

struct SymbolicAccess
{
    static auto make(symbolic::NodePtr root, std::vector<std::string> variables, std::string text, SymbolicLimits limits,
                     std::shared_ptr<void> engine) -> SymbolicExpression
    {
        SymbolicExpression e;
        e.root_ = std::move(root);
        e.variables_ = std::move(variables);
        e.text_ = std::move(text);
        e.limits_ = limits;
        e.engine_ = std::move(engine);
        return e;
    }
    static auto engine(const SymbolicExpression& e) -> const std::shared_ptr<void>& { return e.engine_; }
    static auto limits(const SymbolicExpression& e) -> const SymbolicLimits& { return e.limits_; }
};

#ifdef TFUN_WITH_SYMENGINE

namespace {

using SymEngine::RCP;
using SymEngine::Basic;
using symbolic::Node;
using symbolic::NodePtr;

// SymEngine reads E, I, pi and a few others as constants. A variable with such a name is renamed for the parser.
auto isReserved(const std::string& name) -> bool
{
    static const std::set<std::string> reserved = {"E", "e", "I", "pi", "oo", "zoo", "nan", "EulerGamma", "GoldenRatio", "Catalan"};
    return reserved.count(name) > 0;
}

auto internalName(const std::string& name) -> std::string
{
    return isReserved(name) ? "tfun_" + name : name;
}

struct Counter
{
    int operations = 0;
    int deepest = 0;
};

auto node(Node::Op op) -> std::shared_ptr<Node>
{
    auto n = std::make_shared<Node>();
    n->op = op;
    return n;
}

auto convert(const RCP<const Basic>& e, const std::vector<std::string>& variables, const SymbolicLimits& limits, Counter& count,
             int depth) -> NodePtr
{
    count.deepest = std::max(count.deepest, depth);
    if (depth > limits.max_depth)
        throw SymbolicError("V2-SYM-006", "The expression is deeper than " + std::to_string(limits.max_depth) + " levels.");
    if (++count.operations > limits.max_operations)
        throw SymbolicError("V2-SYM-006", "The expression has more than " + std::to_string(limits.max_operations) + " operations.");

    auto sub = [&](const RCP<const Basic>& x) { return convert(x, variables, limits, count, depth + 1); };
    auto constant = [](double v) {
        auto n = node(Node::Op::Constant);
        n->value = v;
        return NodePtr(n);
    };

    if (SymEngine::is_a_Number(*e))
        return constant(SymEngine::eval_double(*e));
    if (SymEngine::is_a<SymEngine::Constant>(*e))
    {
        if (SymEngine::eq(*e, *SymEngine::E))
            return constant(std::exp(1.0));
        if (SymEngine::eq(*e, *SymEngine::pi))
            return constant(std::acos(-1.0));
        throw SymbolicError("V2-SYM-001", "The constant " + e->__str__() + " is not allowed.");
    }
    if (SymEngine::is_a<SymEngine::Symbol>(*e))
    {
        const std::string name = e->__str__();
        auto it = std::find_if(variables.begin(), variables.end(), [&](const std::string& v) { return internalName(v) == name; });
        if (it == variables.end())
            throw SymbolicError("V2-SYM-001", "The name " + name + " is not a variable of the expression.");
        auto n = node(Node::Op::Variable);
        n->variable = static_cast<int>(it - variables.begin());
        return n;
    }
    if (SymEngine::is_a<SymEngine::Add>(*e) || SymEngine::is_a<SymEngine::Mul>(*e))
    {
        const bool add = SymEngine::is_a<SymEngine::Add>(*e);
        auto n = node(add ? Node::Op::Add : Node::Op::Mul);
        for (const auto& arg : e->get_args())
            n->children.push_back(sub(arg));
        return n;
    }
    if (SymEngine::is_a<SymEngine::Pow>(*e))
    {
        const auto& p = SymEngine::down_cast<const SymEngine::Pow&>(*e);
        if (SymEngine::eq(*p.get_base(), *SymEngine::E))
        {
            auto n = node(Node::Op::Exp);
            n->children.push_back(sub(p.get_exp()));
            return n;
        }
        auto n = node(Node::Op::Pow);
        n->children.push_back(sub(p.get_base()));
        n->children.push_back(sub(p.get_exp()));
        return n;
    }
    if (SymEngine::is_a<SymEngine::Log>(*e))
    {
        auto n = node(Node::Op::Log);
        n->children.push_back(sub(e->get_args()[0]));
        return n;
    }
    if (SymEngine::is_a<SymEngine::Abs>(*e))
    {
        auto n = node(Node::Op::Abs);
        n->children.push_back(sub(e->get_args()[0]));
        return n;
    }
    throw SymbolicError("V2-SYM-001", "The function or construct " + e->__str__() + " is not allowed.");
}

auto build(const RCP<const Basic>& e, const std::string& text, const std::vector<std::string>& variables, const SymbolicLimits& limits)
    -> SymbolicExpression
{
    Counter count;
    NodePtr root = convert(e, variables, limits, count, 0);
    return SymbolicAccess::make(root, variables, text, limits, std::make_shared<RCP<const Basic>>(e));
}

}

auto SymbolicExpression::available() -> bool
{
    return true;
}

auto SymbolicExpression::parse(const std::string& text, const std::vector<std::string>& variables, const SymbolicLimits& limits)
    -> SymbolicExpression
{
    if (text.size() > limits.max_characters)
        throw SymbolicError("V2-SYM-006", "The expression has more than " + std::to_string(limits.max_characters) + " characters.");
    std::string renamed = text;
    for (const auto& v : variables)
        if (isReserved(v))
            renamed = std::regex_replace(renamed, std::regex("\\b" + v + "\\b"), internalName(v));
    RCP<const Basic> e;
    try
    {
        e = SymEngine::parse(renamed);
    }
    catch (const std::exception& error)
    {
        throw SymbolicError("V2-SYM-002", "The expression does not parse: " + std::string(error.what()));
    }
    return build(e, text, variables, limits);
}

auto SymbolicExpression::diff(const std::string& variable) const -> SymbolicExpression
{
    const auto& e = *static_cast<const RCP<const Basic>*>(SymbolicAccess::engine(*this).get());
    RCP<const Basic> d = e->diff(SymEngine::symbol(internalName(variable)));
    return build(d, d->__str__(), variables_, limits_);
}

#else

auto SymbolicExpression::available() -> bool
{
    return false;
}

auto SymbolicExpression::parse(const std::string&, const std::vector<std::string>&, const SymbolicLimits&) -> SymbolicExpression
{
    throw SymbolicError("V2-MTH-008", "This build of ThermoFun has no symbolic methods. Build with TFUN_USE_SYMENGINE=ON.");
}

auto SymbolicExpression::diff(const std::string&) const -> SymbolicExpression
{
    throw SymbolicError("V2-MTH-008", "This build of ThermoFun has no symbolic methods. Build with TFUN_USE_SYMENGINE=ON.");
}

#endif

}
