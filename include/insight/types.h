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
#ifndef INSIGHT_TYPES_H
# define INSIGHT_TYPES_H

# ifdef __cplusplus
namespace Insight {
    class FieldInfo;
    class MethodInfo;
    class StructMemberInfo;
    class StructInfo;
    class PrimitiveTypeInfo;
    class TypeInfo;
    class Container;
}

typedef const Insight::FieldInfo *insight_field_info;
typedef const Insight::MethodInfo *insight_method_info;
typedef const Insight::StructMemberInfo *insight_member_info;
typedef const Insight::StructInfo *insight_struct_info;
typedef const Insight::PrimitiveTypeInfo *insight_primitive_info;
typedef const Insight::TypeInfo *insight_type_info;
typedef const Insight::Container *insight_container_info;
# else
struct insight_field_info_s;
struct insight_method_info_s;
struct insight_member_info_s;
struct insight_struct_info_s;
struct insight_primitive_info_s;
struct insight_type_info_s;
struct insight_container_info_s;

typedef struct insight_field_info_s *insight_field_info;
typedef struct insight_method_info_s *insight_method_info;
typedef struct insight_member_info_s *insight_member_info;
typedef struct insight_struct_info_s *insight_struct_info;
typedef struct insight_primitive_info_s *insight_primitive_info;
typedef struct insight_type_info_s *insight_type_info;
typedef struct insight_container_info_s *insight_container_info;
# endif

typedef void (*insight_field_iter_handle)(insight_field_info field);
typedef void (*insight_method_iter_handle)(insight_method_info method);
typedef void (*insight_type_iter_handle)(insight_type_info type);

typedef enum {
    INSIGHT_KIND_UNKNOWN,
    INSIGHT_KIND_PRIMITIVE,
    INSIGHT_KIND_STRUCT,
    INSIGHT_KIND_UNION,
    INSIGHT_KIND_ENUM,
} e_insight_type_kind;

#endif /* !INSIGHT_TYPES_H */
