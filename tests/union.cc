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
