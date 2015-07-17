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

    Insight::StructInfo& type = dynamic_cast<Insight::StructInfo&>(type_of(Base));

    ASSERT_EQ(n.foo(42), type.method("foo").call<long>(n, 42));
    ASSERT_EQ(s.foo(42), type.method("foo").call<long>(s, 42));
}

