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
