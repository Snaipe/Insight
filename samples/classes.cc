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

int main(int argc, const char *argv[]) {
    Insight::initialize();

    TestClass instance(24);

    std::cout << type_of(42 * 3. + 2).name() << std::endl;

    const Insight::TypeInfo& type = type_of(instance);
    if (auto* clazz = dynamic_cast<const Insight::StructInfo*>(&type)) {
        // access and set the field
        std::cout << "Before: field = " << instance.get_field() << std::endl;
        clazz->field("field").set(instance, 42);
        std::cout << "After: field = " << instance.get_field() << std::endl;

        // retrieve and call a method
        std::cout << "get_field() -> " << clazz->method("get_field").call<int>(instance) << std::endl;
    }

    return 0;
}
