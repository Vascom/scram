/*
 * Copyright (C) 2018 Olzhas Rakhimov
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

/// @file
/// Grouping of event-tree sequences into consequences.

#pragma once

#include <variant>
#include <vector>

#include "element.h"
#include "event_tree.h"

namespace scram::mef {

/// The consequence (effect) of an event-tree sequence.
class Consequence : public Element {
 public:
  /// Type string for error messages.
  static constexpr const char* kTypeString = "consequence";

  /// @copydoc Element::Element
  ///
  /// @param[in] event  The initiating event with the event tree.
  /// @param[in] sequence  The end sequence of the event tree.
  ///
  /// @throws ValidityError  The initiating event does not have an event tree.
  /// @throws ValidityError  The sequence does not belong to the event tree.
  Consequence(std::string name, InitiatingEvent* event, Sequence* sequence);

  /// @returns The initiating event of the event tree.
  /// @{
  InitiatingEvent& initiating_event() { return event_; }
  const InitiatingEvent& initiating_event() const { return event_; }
  /// @}

  /// @returns The end sequence of the event tree.
  /// @{
  Sequence& sequence() { return sequence_; }
  const Sequence& sequence() const { return sequence_; }
  /// @}

 private:
  InitiatingEvent& event_;  ///< The initiating event w/ the event tree.
  Sequence& sequence_;  ///< The sequence in the event tree.
};

/// The group of consequences.
class ConsequenceGroup : public Element {
 public:
  /// Type string for error messages.
  static constexpr const char* kTypeString = "consequence group";

  /// The group member types.
  using Member = std::variant<Consequence*, ConsequenceGroup*>;

  /// The container type for unique consequences as group members.
  using MemberSet = std::vector<Member>;

  using Element::Element;

  /// @returns The group members (consequences).
  const MemberSet& members() const { return members_; }

  /// Adds a member to the consequence group.
  ///
  /// @param[in] member  Another consequence or sub-group.
  ///
  /// @throws DuplicateElementError  The member is a duplicate.
  void Add(Member member);

 private:
  /// Member consequences of this group.
  std::vector<std::variant<Consequence*, ConsequenceGroup*>> members_;
};

}  // namespace scram::mef
