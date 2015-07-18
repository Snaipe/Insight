#include <insight/insight>
#include <iostream>

class TestClass {
public:
    TestClass(int f);
    int get_field();
private:
    int field;
};

TestClass::TestClass(int f)
    : field(f)
{}

int TestClass::get_field() {
    return field;
}

int main(void) {
    TestClass instance(24);

    Insight::StructInfo& type = type_of(TestClass);

    // access and set the field
    std::cout << "Before: field = " << instance.get_field() << std::endl;
    type.field("field").set(instance, 42);
    std::cout << "After: field = " << instance.get_field() << std::endl;

    // alternative way
    int& field = type.field("field").get<int>(instance);
    std::cout << "Field reference access: field = " << field << std::endl;
    field = 55;

    // retrieve and call a method
    std::cout << "get_field() -> " << type.method("get_field").call<int>(instance) << std::endl;

    return 0;
}
