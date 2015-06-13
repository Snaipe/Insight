#include <insight/insight.h>
#include <stdio.h>

struct test_struct {
    int field;
};

int main(int argc, const char *argv[]) {
    insight_initialize();

    struct test_struct t = {24};

    const insight_struct_info type = (const insight_struct_info) insight_type_of(test_struct);
    const insight_field_info field = insight_field(type, "field");

    printf("%d\n", t.field);
    int newdata = 42;
    insight_field_set(field, &t, &newdata, sizeof(newdata));
    printf("%d\n", t.field);

    return 0;
}

