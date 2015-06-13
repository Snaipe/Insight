#ifndef INSIGHT_INSIGHT_H
# define INSIGHT_INSIGHT_H

# include <stddef.h>
# include "types.h"

# define insight_type_of(Type) insight_type_of_str(#Type)

void insight_initialize(void);
insight_type_info insight_type_of_str(const char *);
insight_field_info insight_field(insight_struct_info info, const char *name);
void insight_field_set(insight_field_info info, void *instance, void *data, size_t datasize);
void insight_field_get(insight_field_info info, void *instance, void *data, size_t datasize);

#endif /* !INSIGHT_INSIGHT_H */
