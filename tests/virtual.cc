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

class Base {
public:
    virtual long foo(int x) const = 0;
};

class Neg : public Base {
public:
    virtual long foo(int x) const override;
};

long Neg::foo(int x) const {
    return -x;
}

class Square : public Base {
public:
    virtual long foo(int x) const override;
};

long Square::foo(int x) const {
    return x * x;
}

TEST(Virtual, Call) {
    Neg n;
    Square s;

    Insight::StructInfo& type = type_of(Base);

    ASSERT_EQ(n.foo(42), type.method("foo").call<long>(n, 42));
    ASSERT_EQ(s.foo(42), type.method("foo").call<long>(s, 42));
}

