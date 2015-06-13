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

struct insight_field_info_s;
struct insight_method_info_s;
struct insight_member_info_s;
struct insight_struct_info_s;
struct insight_primitive_info_s;
struct insight_type_info_s;

typedef struct insight_field_info_s *insight_field_info;
typedef struct insight_method_info_s *insight_method_info;
typedef struct insight_member_info_s *insight_member_info;
typedef struct insight_struct_info_s *insight_struct_info;
typedef struct insight_primitive_info_s *insight_primitive_info;
typedef struct insight_type_info_s *insight_type_info;

#endif /* !INSIGHT_TYPES_H */
