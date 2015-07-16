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
#ifndef INSIGHT_INSIGHT_H
# define INSIGHT_INSIGHT_H

# include <stddef.h>
# include "types.h"

# if defined(__GNUC__)
#  define insight_type_of(Thing) ({                                                         \
        static __typeof__(Thing) *insight_typeof_dummy __attribute__((used)) = (void*)0;    \
        insight_type_of_addr(&insight_typeof_dummy);                                        \
    })
# else
#  define insight_type_of(Type) insight_type_of_str(#Type)
# endif

# ifndef type_of
#  define type_of insight_type_of
# endif

insight_type_info insight_type_of_str(const char *);
insight_type_info insight_type_of_addr(void *);

e_insight_type_kind insight_type_kind(insight_type_info type);
const char *insight_type_name(insight_type_info type);

void insight_iter_fields(insight_struct_info type, insight_field_iter_handle handle);
void insight_iter_methods(insight_struct_info type, insight_field_iter_handle handle);
void insight_iter_types(insight_container_info type, insight_field_iter_handle handle);

insight_field_info insight_field(insight_struct_info info, const char *name);
void insight_field_set(insight_field_info info, void *instance, void *data, size_t datasize);
void insight_field_get(insight_field_info info, void *instance, void *data, size_t datasize);

#endif /* !INSIGHT_INSIGHT_H */
