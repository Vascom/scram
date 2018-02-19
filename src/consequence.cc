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
/// Implementation of Consequence and Consequence groups.

#include "consequence.h"

#include "error.h"

#include "ext/algorithm.h"
#include "ext/variant.h"

namespace scram::mef {

Consequence::Consequence(std::string name, InitiatingEvent* event,
                         Sequence* sequence)
    : Element(std::move(name)), event_(*event), sequence_(*sequence) {
  if (!event_.event_tree())
    SCRAM_THROW(ValidityError("No event-tree for initiating event"))
        << errinfo_element(event_.name(), "initiating event");

  if (event_.event_tree()->sequences().count(sequence_.name()) == false)
    SCRAM_THROW(ValidityError("The sequence does not belong to the event tree"))
        << errinfo_element(sequence_.name(), "sequence")
        << errinfo_container(event_.event_tree()->name(), "event tree");
}

void ConsequenceGroup::Add(Member member) {
  Element* element = ext::as<Element*>(member);
  bool is_duplicate =
      ext::any_of(members_, [&member, &element](const Member& current) {
        return current.index() == member.index() &&
               element->name() == ext::as<Element*>(current)->name();
      });

  if (is_duplicate)
    SCRAM_THROW(DuplicateElementError())
        << errinfo_element(element->name(), "consequence group member")
        << errinfo_container(Element::name(), "consequence group");
}

}  // namespace scram::mef
