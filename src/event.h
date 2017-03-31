/*
 * Copyright (C) 2014-2017 Olzhas Rakhimov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/// @file event.h
/// Contains event classes for fault trees.

#ifndef SCRAM_SRC_EVENT_H_
#define SCRAM_SRC_EVENT_H_

#include <cstdint>

#include <memory>
#include <string>
#include <vector>

#include <boost/noncopyable.hpp>

#include "element.h"
#include "error.h"
#include "expression.h"

namespace scram {
namespace mef {

/// Abstract base class for general fault tree events.
class Event : public Id, private boost::noncopyable {
 public:
  using Id::Id;

  virtual ~Event() = 0;  ///< Abstract class.

  /// @returns True if this node is orphan.
  bool orphan() const { return orphan_; }

  /// Sets the orphan state.
  ///
  /// @param[in] state  True if this event is not used anywhere.
  void orphan(bool state) { orphan_ = state; }

 private:
  bool orphan_ = true;  ///< Indication of an orphan node.
};

/// Representation of a house event in a fault tree.
///
/// @note House Events with unset/uninitialized expressions default to False.
class HouseEvent : public Event {
 public:
  using Event::Event;

  /// Sets the state for House event.
  ///
  /// @param[in] constant  False or True for the state of this house event.
  void state(bool constant) {
    state_ = constant;
  }

  /// @returns The true or false state of this house event.
  bool state() const { return state_; }

 private:
  /// Represents the state of the house event.
  /// Implies On or Off for True or False values of the probability.
  bool state_ = false;
};

class Gate;
using GatePtr = std::shared_ptr<Gate>;  ///< Shared gates in models.

/// Representation of a basic event in a fault tree.
class BasicEvent : public Event {
 public:
  using Event::Event;

  virtual ~BasicEvent() = default;

  /// @returns true if the probability expression is set.
  bool HasExpression() const { return expression_ != nullptr; }

  /// Sets the expression of this basic event.
  ///
  /// @param[in] expression  The expression to describe this event.
  void expression(const ExpressionPtr& expression) {
    assert(!expression_ && "The basic event's expression is already set.");
    expression_ = expression;
  }

  /// @returns The previously set expression for analysis purposes.
  ///
  /// @pre The expression has been set.
  Expression& expression() const {
    assert(expression_ && "The basic event's expression is not set.");
    return *expression_;
  }

  /// @returns The mean probability of this basic event.
  ///
  /// @note The user of this function should make sure
  ///       that the returned value is acceptable for calculations.
  ///
  /// @warning Undefined behavior if the expression is not set.
  double p() const noexcept {
    assert(expression_ && "The basic event's expression is not set.");
    return expression_->Mean();
  }

  /// Validates the probability expressions for the primary event.
  ///
  /// @pre The probability expression is set.
  ///
  /// @throws ValidationError  The expression for the basic event is invalid.
  void Validate() const {
    assert(expression_ && "The basic event's expression is not set.");
    if (expression_->Min() < 0 || expression_->Max() > 1) {
      throw ValidationError("Expression value is invalid.");
    }
  }

  /// Indicates if this basic event has been set to be in a CCF group.
  ///
  /// @returns true if in a CCF group.
  bool HasCcf() const { return ccf_gate_ != nullptr; }

  /// @returns CCF group gate representing this basic event.
  const Gate& ccf_gate() const {
    assert(ccf_gate_);
    return *ccf_gate_;
  }

  /// Sets the common cause failure group gate
  /// that can represent this basic event
  /// in analysis with common cause information.
  /// This information is expected to be provided by
  /// CCF group application.
  ///
  /// @param[in] gate  CCF group gate.
  void ccf_gate(std::unique_ptr<Gate> gate) {
    assert(!ccf_gate_);
    ccf_gate_ = std::move(gate);
  }

 private:
  /// Expression that describes this basic event
  /// and provides numerical values for probability calculations.
  ExpressionPtr expression_;

  /// If this basic event is in a common cause group,
  /// CCF gate can serve as a replacement for the basic event
  /// for common cause analysis.
  std::unique_ptr<Gate> ccf_gate_;
};

using EventPtr = std::shared_ptr<Event>;  ///< Base shared pointer for events.
using HouseEventPtr = std::shared_ptr<HouseEvent>;  ///< Shared house events.
using BasicEventPtr = std::shared_ptr<BasicEvent>;  ///< Shared basic events.

class Formula;  // To describe a gate's formula.
using FormulaPtr = std::unique_ptr<Formula>;  ///< Non-shared gate formulas.

class Initializer;  // Needs to handle cycles with gates.

/// A representation of a gate in a fault tree.
class Gate : public Event, public NodeMark {
 public:
  /// Provides access to cycle-destructive functions.
  class Cycle {
    friend class Initializer;  // Only Initializer needs the functionality.
    /// Breaks connections in a fault tree.
    ///
    /// @param[in,out] gate  A gate in a cycle or potentially in a cycle.
    ///
    /// @post The fault tree is unusable for analysis.
    ///       Only destruction is guaranteed to succeed.
    static void BreakConnections(Gate* gate) {
      gate->formula_.reset();
    }
  };

  using Event::Event;

  /// @returns The formula of this gate.
  /// @{
  const Formula& formula() const { return *formula_; }
  Formula& formula() { return *formula_; }
  /// @}

  /// Sets the formula of this gate.
  ///
  /// @param[in] formula  Boolean formula of this gate.
  void formula(FormulaPtr formula) {
    assert(!formula_);
    formula_ = std::move(formula);
  }

  /// Checks if a gate is initialized correctly.
  ///
  /// @throws ValidationError  Errors in the gate's logic or setup.
  void Validate() const;

 private:
  FormulaPtr formula_;  ///< Boolean formula of this gate.
};

/// Operators for formulas.
/// The ordering is the same as analysis operators in the PDAG.
enum Operator : std::uint8_t {
  kAnd = 0,
  kOr,
  kVote,  ///< Combination, K/N, atleast, or Vote gate representation.
  kXor,  ///< Exclusive OR gate with two inputs only.
  kNot,  ///< Boolean negation.
  kNand,  ///< Not AND.
  kNor,  ///< Not OR.
  kNull  ///< Single argument pass-through without logic.
};

/// The number of operators in the enum.
const int kNumOperators = 8;

/// String representations of the operators.
/// The ordering is the same as the Operator enum.
const char* const kOperatorToString[] = {"and", "or",   "atleast", "xor",
                                         "not", "nand", "nor",     "null"};

/// Boolean formula with operators and arguments.
/// Formulas are not expected to be shared.
class Formula : private boost::noncopyable {
 public:
  /// Constructs a formula.
  ///
  /// @param[in] type  The logical operator for this Boolean formula.
  explicit Formula(Operator type);

  /// @returns The type of this formula.
  Operator type() const { return type_; }

  /// @returns The vote number if and only if the formula is "atleast".
  ///
  /// @throws LogicError  The vote number is not yet assigned.
  int vote_number() const;

  /// Sets the vote number only for an "atleast" formula.
  ///
  /// @param[in] number  The vote number.
  ///
  /// @throws InvalidArgument  The vote number is invalid.
  /// @throws LogicError  The vote number is assigned illegally.
  ///
  /// @note (Children number > vote number) should be checked
  ///       outside of this class.
  void vote_number(int number);

  /// @returns The arguments of this formula of specific type.
  /// @{
  const IdTable<Event*>& event_args() const { return event_args_; }
  const std::vector<HouseEventPtr>& house_event_args() const {
    return house_event_args_;
  }
  const std::vector<BasicEventPtr>& basic_event_args() const {
    return basic_event_args_;
  }
  const std::vector<GatePtr>& gate_args() const { return gate_args_; }
  const std::vector<FormulaPtr>& formula_args() const { return formula_args_; }
  /// @}

  /// @returns The number of arguments.
  int num_args() const { return event_args_.size() + formula_args_.size(); }

  /// Adds an event into the arguments list.
  ///
  /// @param[in] event  A pointer to an argument event.
  ///
  /// @throws DuplicateArgumentError  The argument event is duplicate.
  ///
  /// @{
  void AddArgument(const HouseEventPtr& event) {
    AddArgument(event, &house_event_args_);
  }
  void AddArgument(const BasicEventPtr& event) {
    AddArgument(event, &basic_event_args_);
  }
  void AddArgument(const GatePtr& event) {
    AddArgument(event, &gate_args_);
  }
  /// @}

  /// Adds a formula into the arguments list.
  /// Formulas are unique.
  ///
  /// @param[in] formula  A pointer to an argument formula.
  void AddArgument(FormulaPtr formula) {
    formula_args_.emplace_back(std::move(formula));
  }

  /// Checks if a formula is initialized correctly with the number of arguments.
  ///
  /// @throws ValidationError  Problems with the operator or arguments.
  void Validate() const;

 private:
  /// Handles addition of an event to the formula.
  ///
  /// @tparam Ptr  Shared pointer type to the event.
  ///
  /// @param[in] event  Pointer to the event.
  /// @param[in,out] container  The final destination to save the event.
  ///
  /// @throws DuplicateArgumentError  The argument event is duplicate.
  template <class Ptr>
  void AddArgument(const Ptr& event, std::vector<Ptr>* container) {
    if (event_args_.insert(event.get()).second == false)
      throw DuplicateArgumentError("Duplicate argument " + event->name());
    container->emplace_back(event);
    if (event->orphan())
      event->orphan(false);
  }

  Operator type_;  ///< Logical operator.
  int vote_number_;  ///< Vote number for "atleast" operator.
  IdTable<Event*> event_args_;  ///< All event arguments.
  std::vector<HouseEventPtr> house_event_args_;  ///< House event arguments.
  std::vector<BasicEventPtr> basic_event_args_;  ///< Basic event arguments.
  std::vector<GatePtr> gate_args_;  ///< Arguments that are gates.
  /// Arguments that are formulas
  /// if this formula is nested.
  std::vector<FormulaPtr> formula_args_;
};

}  // namespace mef
}  // namespace scram

#endif  // SCRAM_SRC_EVENT_H_
