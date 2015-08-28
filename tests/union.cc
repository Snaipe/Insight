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

union UnionTest {
    int foo;
    char bar;
};

TEST(Union, Field) {
    UnionTest instance = {0};

    auto& type = type_of(instance);
    auto& field = type.field("foo");

    int& value = field.get<int>(instance);
    value = 42;
    EXPECT_EQ(instance.foo, value);

    value = 42;
    EXPECT_EQ(42, instance.foo);

    field.set(instance, 24);
    EXPECT_EQ(24, instance.foo);

}
