#include "insight/insight"
#include <cstring>
#include <cassert>

extern "C" {

#include "insight/types.h"

void insight_initialize(void) {
    Insight::initialize();
}

insight_type_info insight_type_of_str(const char *name) {
    try {
        return reinterpret_cast<insight_type_info>(const_cast<Insight::TypeInfo*>(&Insight::type_of(name)));
    } catch (std::out_of_range &ex) {
        return NULL;
    }
}

insight_field_info insight_field(insight_struct_info info, const char *name) {
    Insight::StructInfo* sinfo = reinterpret_cast<Insight::StructInfo*>(info);
    try {
        return reinterpret_cast<insight_field_info>(const_cast<Insight::FieldInfo*>(&sinfo->field(name)));
    } catch (std::out_of_range &ex) {
        return NULL;
    }
}

void insight_field_set(insight_field_info info, void *instance, void *data, size_t datasize) {
    Insight::FieldInfo* finfo = reinterpret_cast<Insight::FieldInfo*>(info);
    assert(finfo->type().size_of() == datasize);
    void* field = static_cast<char*>(instance) + finfo->offset();
    std::memcpy(field, data, datasize);
}

void insight_field_get(insight_field_info info, void *instance, void *data, size_t datasize) {
    Insight::FieldInfo* finfo = reinterpret_cast<Insight::FieldInfo*>(info);
    assert(finfo->type().size_of() == datasize);
    void* field = static_cast<char*>(instance) + finfo->offset();
    std::memcpy(data, field, datasize);
}

}
