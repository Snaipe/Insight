#ifndef INSIGHT_UNION_HH
# define INSIGHT_UNION_HH

# include "type.hh"

namespace Insight {

    std::shared_ptr<TypeInfo> build_union_type(Dwarf::Die &die, BuildContext& ctx, bool register_parent);

}

#endif /* !INSIGHT_UNION_HH */
