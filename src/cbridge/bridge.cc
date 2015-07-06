/*
 * This file is part of Insight.
 *
 * Copyright © 2015 Franklin "Snaipe" Mathieu <http://snaipe.me>
 *
 * Insight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Insight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Insight.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "insight/insight"
#include <cstring>
#include <cassert>

extern "C" {

namespace Insight {
    struct FieldInfo;
    struct MethodInfo;
    struct MemberInfo;
    struct StructInfo;
    struct PrimitiveInfo;
    struct TypeInfo;
}

typedef const Insight::FieldInfo *insight_field_info;
typedef const Insight::MethodInfo *insight_method_info;
typedef const Insight::MemberInfo *insight_member_info;
typedef const Insight::StructInfo *insight_struct_info;
typedef const Insight::PrimitiveTypeInfo *insight_primitive_info;
typedef const Insight::TypeInfo *insight_type_info;

void insight_initialize(void) {
    Insight::initialize();
}

insight_type_info insight_type_of_str(const char *name) {
    try {
        return &Insight::type_of(name);
    } catch (std::out_of_range &ex) {
        return NULL;
    }
}

const char *insight_type_name(insight_type_info info) {
    return info->name().c_str();
}

insight_field_info insight_field(insight_struct_info info, const char *name) {
    try {
        return &info->field(name);
    } catch (std::out_of_range &ex) {
        return NULL;
    }
}

void insight_field_set(insight_field_info info, void *instance, void *data, size_t datasize) {
    assert(info->type().size_of() == datasize);
    void* field = static_cast<char*>(instance) + info->offset();
    std::memcpy(field, data, datasize);
}

void insight_field_get(insight_field_info info, void *instance, void *data, size_t datasize) {
    assert(info->type().size_of() == datasize);
    void* field = static_cast<char*>(instance) + info->offset();
    std::memcpy(data, field, datasize);
}

}
