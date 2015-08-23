#include <gtest/gtest.h>
#include "insight/insight"

using namespace Insight;

enum Color {
    red, blue, green
};

TEST(Enum, Simple) {
    Color instance;

    auto& type = type_of(instance);

    EnumConstantInfo& redInfo = type.value("red");
    EXPECT_EQ(red, redInfo.get<Color>());
}
