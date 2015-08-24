#ifndef INSIGHT_STRUCT_HH
# define INSIGHT_STRUCT_HH

# include "type.hh"

namespace Insight {

    std::shared_ptr<TypeInfo> build_struct_type(Dwarf::Die &die, BuildContext& ctx, bool register_parent);

}

#endif /* !INSIGHT_STRUCT_HH */
