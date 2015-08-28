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
