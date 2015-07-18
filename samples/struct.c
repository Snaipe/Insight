#include <insight/insight.h>
#include <stdio.h>

struct test_struct {
    int field;
};

int main(void) {
    struct test_struct t = {24};

    insight_struct_info type = (insight_struct_info) type_of(struct test_struct);
    insight_field_info field = insight_field(type, "field");

    printf("%d\n", t.field);
    int newdata = 42;
    insight_field_set(field, &t, &newdata, sizeof(newdata));
    printf("%d\n", t.field);

    return 0;
}

