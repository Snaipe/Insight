/*
 * This file is part of Insight.
 *
 * Copyright © 2015 Franklin "Snaipe" Mathieu <http://snaipe.me>
 *
 * Insight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Insight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Insight.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <gtest/gtest.h>
#include "insight/insight"

using namespace Insight;

class ClassTest {
public:
    ClassTest();
    int get_bar() const;
    void set_bar(int bar);
private:
    int bar;
};

ClassTest::ClassTest() : bar(0) {}

int ClassTest::get_bar() const {
    return bar;
}

void ClassTest::set_bar(int bar) {
    this->bar = bar;
}

TEST(Class, Field) {
    ClassTest instance;

    auto& type = type_of(instance);
    auto& field = type.field("bar");

    int& value = field.get<int>(instance);
    EXPECT_EQ(instance.get_bar(), value);

    value = 42;
    EXPECT_EQ(42, instance.get_bar());

    field.set(instance, 24);
    EXPECT_EQ(24, instance.get_bar());
}

TEST(Class, Method) {
    ClassTest instance;

    auto& type = type_of(instance);
    auto& get_bar = type.method("get_bar");
    auto& set_bar = type.method("set_bar");

    instance.set_bar(42);
    EXPECT_EQ(42, get_bar.call<int>(instance));

    set_bar.call<void>(instance, 24);
    EXPECT_EQ(24, instance.get_bar());
}
