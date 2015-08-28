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
#include "insight/annotate"

using namespace Insight;

insight_annotation(Valued) {
    int value;
};

$(Valued, .value = 1)
namespace Foo {
    int dummy;
}

$(Valued, .value = 2)
class FooClass {
public:

    $(Valued, .value = 3)
    int field;

    $(Valued, .value = 4)
    void method();
};

void FooClass::method() {
}

$(Valued, .value = 5)
union FooUnion {
};

TEST(Annotation, Namespace) {
    Foo::dummy = 42;

    NamespaceInfo& ns = Insight::root_namespace().nested_namespace("Foo");

    AnnotationInfo& annotation = ns.annotation("Valued");
    ASSERT_EQ(1, annotation.data<const Valued>().value);
}

TEST(Annotation, Class) {
    StructInfo& info = type_of(FooClass);
    AnnotationInfo& sa = info.annotation("Valued");

    FieldInfo& field = info.field("field");
    AnnotationInfo& fa = field.annotation("Valued");

    MethodInfo& method = info.method("method");
    AnnotationInfo& ma = method.annotation("Valued");

    ASSERT_EQ(2, sa.data<const Valued>().value);
    ASSERT_EQ(3, fa.data<const Valued>().value);
    ASSERT_EQ(4, ma.data<const Valued>().value);
}

TEST(Annotation, Union) {
    UnionInfo& info = type_of(FooUnion);

    AnnotationInfo& annotation = info.annotation("Valued");
    ASSERT_EQ(5, annotation.data<const Valued>().value);
}
