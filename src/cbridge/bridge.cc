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

#include "insight/types.h"

e_insight_type_kind insight_type_kind(insight_type_info type) {
    if (dynamic_cast<const Insight::StructInfo*>(type)) {
        return INSIGHT_KIND_STRUCT;
    } else if (dynamic_cast<const Insight::PrimitiveTypeInfo*>(type)) {
        return INSIGHT_KIND_PRIMITIVE;
    }
    return INSIGHT_KIND_UNKNOWN;
}

void insight_initialize(void) {
    Insight::initialize();
}

insight_type_info insight_type_of_str(const char *name) {
    try {
        return &Insight::type_of_(name);
    } catch (std::out_of_range &ex) {
        return NULL;
    }
}

insight_type_info insight_type_of_addr(void *addr) {
    try {
        return &Insight::type_of_(addr);
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

void insight_iter_fields(insight_struct_info type, insight_field_iter_handle handle) {
    for (auto& field : type->fields())
        handle(&field);
}

void insight_iter_methods(insight_struct_info type, insight_method_iter_handle handle) {
    for (auto& method : type->methods())
        handle(&method);
}

void insight_iter_types(insight_container_info type, insight_type_iter_handle handle) {
    for (auto& t : type->types())
        handle(&t);
}

}
