/*
 * Copyright (C) 2014-2018 Olzhas Rakhimov
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

#include "element.h"

#include <gtest/gtest.h>

#include "error.h"

namespace scram::mef::test {

namespace {

class NamedElement : public Element {
 public:
  using Element::Element;
};

}  // namespace

TEST(ElementTest, Name) {
  EXPECT_THROW(NamedElement(""), LogicError);

  EXPECT_THROW(NamedElement(".name"), ValidityError);
  EXPECT_THROW(NamedElement("na.me"), ValidityError);
  EXPECT_THROW(NamedElement("name."), ValidityError);

  EXPECT_NO_THROW(NamedElement("name"));
  NamedElement el("name");
  EXPECT_EQ("name", el.name());

  // Illegal names by MEF.
  // However, this names don't mess with class and reference invariants.
  EXPECT_NO_THROW(NamedElement("na me"));
  EXPECT_NO_THROW(NamedElement("na\nme"));
  EXPECT_NO_THROW(NamedElement("\tname"));
  EXPECT_NO_THROW(NamedElement("name?"));
}

TEST(ElementTest, Label) {
  NamedElement el("name");
  EXPECT_EQ("", el.label());
  EXPECT_NO_THROW(el.label(""));
  ASSERT_NO_THROW(el.label("label"));
  EXPECT_EQ("label", el.label());
  EXPECT_NO_THROW(el.label("new_label"));
  EXPECT_NO_THROW(el.label(""));
}

TEST(ElementTest, AddAttribute) {
  NamedElement el("name");
  Attribute attr;
  attr.name = "impact";
  attr.value = "0.1";
  attr.type = "float";
  EXPECT_THROW(el.GetAttribute(attr.name), LogicError);
  ASSERT_NO_THROW(el.AddAttribute(attr));
  EXPECT_THROW(el.AddAttribute(attr), DuplicateArgumentError);
  ASSERT_TRUE(el.HasAttribute(attr.name));
  ASSERT_NO_THROW(el.GetAttribute(attr.name));
  EXPECT_EQ(attr.value, el.GetAttribute(attr.name).value);
  EXPECT_EQ(attr.name, el.GetAttribute(attr.name).name);
}

TEST(ElementTest, SetAttribute) {
  NamedElement el("name");
  Attribute attr;
  attr.name = "impact";
  attr.value = "0.1";
  attr.type = "float";
  EXPECT_THROW(el.GetAttribute(attr.name), LogicError);
  ASSERT_NO_THROW(el.SetAttribute(attr));
  EXPECT_THROW(el.AddAttribute(attr), DuplicateArgumentError);
  ASSERT_TRUE(el.HasAttribute(attr.name));
  ASSERT_NO_THROW(el.GetAttribute(attr.name));
  EXPECT_EQ(attr.value, el.GetAttribute(attr.name).value);
  EXPECT_EQ(attr.name, el.GetAttribute(attr.name).name);

  attr.value = "0.2";
  ASSERT_NO_THROW(el.SetAttribute(attr));
  EXPECT_EQ(1, el.attributes().size());
  ASSERT_NO_THROW(el.GetAttribute(attr.name));
  EXPECT_EQ(attr.value, el.GetAttribute(attr.name).value);
}

TEST(ElementTest, RemoveAttribute) {
  NamedElement el("name");
  Attribute attr;
  attr.name = "impact";
  attr.value = "0.1";
  attr.type = "float";

  EXPECT_FALSE(el.HasAttribute(attr.name));
  EXPECT_TRUE(el.attributes().empty());
  EXPECT_FALSE(el.RemoveAttribute(attr.name));

  ASSERT_NO_THROW(el.AddAttribute(attr));
  EXPECT_TRUE(el.RemoveAttribute(attr.name));
  EXPECT_FALSE(el.HasAttribute(attr.name));
  EXPECT_TRUE(el.attributes().empty());
}

namespace {

class TestRole : public Role {
 public:
  using Role::Role;
};

}  // namespace

TEST(ElementTest, Role) {
  EXPECT_THROW(TestRole(RoleSpecifier::kPublic, ".ref"), ValidityError);
  EXPECT_THROW(TestRole(RoleSpecifier::kPublic, "ref."), ValidityError);
  EXPECT_NO_THROW(TestRole(RoleSpecifier::kPublic, "ref.name"));

  EXPECT_THROW(TestRole(RoleSpecifier::kPrivate, ""), ValidityError);
  EXPECT_NO_THROW(TestRole(RoleSpecifier::kPublic, ""));
}

namespace {

class NameId : public Id {
 public:
  using Id::Id;
};

}  // namespace

TEST(ElementTest, Id) {
  EXPECT_THROW(NameId(""), LogicError);
  EXPECT_NO_THROW(NameId("name"));
  EXPECT_THROW(NameId("name", "", RoleSpecifier::kPrivate), ValidityError);

  NameId id_public("name");
  EXPECT_EQ(id_public.id(), id_public.name());

  NameId id_private("name", "path", RoleSpecifier::kPrivate);
  EXPECT_EQ("path.name", id_private.id());
  EXPECT_NE(id_private.id(), id_private.name());

  EXPECT_NE(id_public.id(), id_private.id());

  // Reset.
  id_public.id("id");
  EXPECT_EQ("id", id_public.id());
  EXPECT_EQ("id", id_public.name());
  id_private.id("id");
  EXPECT_EQ("path.id", id_private.id());
  EXPECT_EQ("id", id_private.name());
}

}  // namespace scram::mef::test
