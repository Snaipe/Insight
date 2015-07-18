#include <gtest/gtest.h>
#include "insight/insight"

using namespace Insight;

#define TEST_PRIMITIVE_TYPE(T, Kind) \
    do { \
        PrimitiveTypeInfo& type = type_of(T); \
        EXPECT_EQ(sizeof(T), type.size_of()); \
        EXPECT_EQ(Kind, type.kind()); \
    } while (0)

TEST(Typeof, Pointer) {
    PointerTypeInfo& type = type_of(void*);
    EXPECT_EQ(type.pointed_type(), type_of(void));

    EXPECT_EQ(type_of(void *).size_of(), type_of(int *).size_of());
}

TEST(Typeof, Primitive) {
    TEST_PRIMITIVE_TYPE(int, PrimitiveKind::INT);
    TEST_PRIMITIVE_TYPE(short int, PrimitiveKind::SHORT_INT);
    TEST_PRIMITIVE_TYPE(long int, PrimitiveKind::LONG_INT);
    TEST_PRIMITIVE_TYPE(long long int, PrimitiveKind::LONG_LONG_INT);
    TEST_PRIMITIVE_TYPE(unsigned int, PrimitiveKind::UNSIGNED_INT);
    TEST_PRIMITIVE_TYPE(unsigned short int, PrimitiveKind::UNSIGNED_SHORT_INT);
    TEST_PRIMITIVE_TYPE(unsigned long int, PrimitiveKind::UNSIGNED_LONG_INT);
    TEST_PRIMITIVE_TYPE(unsigned long long int, PrimitiveKind::UNSIGNED_LONG_LONG_INT);
    TEST_PRIMITIVE_TYPE(float, PrimitiveKind::FLOAT);
    TEST_PRIMITIVE_TYPE(double, PrimitiveKind::DOUBLE);
    TEST_PRIMITIVE_TYPE(long double, PrimitiveKind::LONG_DOUBLE);
    TEST_PRIMITIVE_TYPE(char, PrimitiveKind::CHAR);
    TEST_PRIMITIVE_TYPE(signed char, PrimitiveKind::CHAR);
    TEST_PRIMITIVE_TYPE(unsigned char, PrimitiveKind::UNSIGNED_CHAR);
    TEST_PRIMITIVE_TYPE(bool, PrimitiveKind::BOOL);
    TEST_PRIMITIVE_TYPE(float _Complex, PrimitiveKind::FLOAT_COMPLEX);
    TEST_PRIMITIVE_TYPE(double _Complex, PrimitiveKind::DOUBLE_COMPLEX);
    TEST_PRIMITIVE_TYPE(long double _Complex, PrimitiveKind::LONG_DOUBLE_COMPLEX);

    // Test void separately since sizeof(void) is undefined
    PrimitiveTypeInfo& voidtype = type_of(void);
    EXPECT_EQ(0, voidtype.size_of());
    EXPECT_EQ(PrimitiveKind::VOID, voidtype.kind());
}

TEST(Typeof, Variants) {
    EXPECT_EQ(type_of(signed), type_of(int));
    EXPECT_EQ(type_of(signed), type_of(signed int));
    EXPECT_EQ(type_of(unsigned), type_of(unsigned int));

    EXPECT_EQ(type_of(signed int), type_of(int));
    EXPECT_EQ(type_of(int signed), type_of(signed int));
    EXPECT_EQ(type_of(int unsigned), type_of(unsigned int));

    EXPECT_EQ(type_of(long unsigned), type_of(unsigned long));
    EXPECT_EQ(type_of(long signed), type_of(signed long));

    EXPECT_EQ(type_of(long long unsigned), type_of(unsigned long long));
    EXPECT_EQ(type_of(long long signed), type_of(signed long long));
    EXPECT_EQ(type_of(long unsigned long), type_of(unsigned long long));
    EXPECT_EQ(type_of(long unsigned long), type_of(long long unsigned));
    EXPECT_EQ(type_of(long signed long), type_of(signed long long));
    EXPECT_EQ(type_of(long signed long), type_of(long long signed));

    // char being signed or not is implementation defined
    EXPECT_NE(type_of(signed char), type_of(char));
    EXPECT_NE(type_of(unsigned char), type_of(char));
    EXPECT_EQ(type_of(unsigned char), type_of(char unsigned));
    EXPECT_EQ(type_of(signed char), type_of(char signed));

    EXPECT_EQ(type_of(long double), type_of(double long));

    EXPECT_EQ(type_of(float _Complex), type_of(_Complex float));
    EXPECT_EQ(type_of(double _Complex), type_of(_Complex double));
    EXPECT_EQ(type_of(long double _Complex), type_of(_Complex long double));
    EXPECT_EQ(type_of(long _Complex double), type_of(_Complex long double));
}

class Foo {
public:
    Foo(int n) : n_(n) {}

    int n_;
};

TEST(Typeof, Expression) {
    EXPECT_EQ(type_of(1), type_of(int));
    EXPECT_EQ(type_of(1u), type_of(unsigned int));
    EXPECT_EQ(type_of(1l), type_of(long));
    EXPECT_EQ(type_of(1ll), type_of(long long));
    EXPECT_EQ(type_of(1ul), type_of(unsigned long));
    EXPECT_EQ(type_of(1ull), type_of(unsigned long long));

    EXPECT_EQ(type_of(1.f), type_of(float));
    EXPECT_EQ(type_of(1.), type_of(double));
    EXPECT_EQ(type_of(1.l), type_of(long double));

    EXPECT_EQ(type_of('1'), type_of(char));
#if __cplusplus >= 201701L // placeholder test for C++17 new literals
    EXPECT_EQ(type_of(u8'1'), type_of(char));
    EXPECT_EQ(type_of(u8'∀'), type_of(int));
#endif
    EXPECT_EQ(type_of(u'1'), type_of(char16_t));
    EXPECT_EQ(type_of(U'1'), type_of(char32_t));
    EXPECT_EQ(type_of(L'1'), type_of(wchar_t));
    EXPECT_EQ(type_of('ABCD'), type_of(int));

    EXPECT_EQ(type_of(true), type_of(bool));
#if __cplusplus >= 201103L
    EXPECT_EQ(type_of(nullptr), type_of(nullptr_t));
    EXPECT_EQ(sizeof(void*), type_of(nullptr_t).size_of());
#endif

    EXPECT_EQ(type_of(Foo(42)), type_of(Foo));
}
